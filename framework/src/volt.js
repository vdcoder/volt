// volt.js
// Tiny bootstrap helper for Volt apps using the Emscripten-generated VoltApp factory.
//
// Assumes that `app.js` has defined a global `VoltApp` function:
//   VoltApp(moduleOverrides) -> Promise<Module>
//
// This file sets up:
//  - console logging
//  - event bubbling from a container to Volt
//  - runtime creation
//  - basic error handling
//
// Usage (in index.html):
//   <script src="app.js"></script>
//   <script src="volt.js"></script>
//   <script>
//     VoltBootstrap.start({
//       containerId: "app-container",
//       rootId: "root",
//       debug: true,
//     });
//   </script>

(function (global) {
  "use strict";

  const DEFAULT_EVENTS = [
    "click",
    "input",
    "change",
    "submit",
    "keydown",
    "keyup",
    "keypress",
    "mousedown",
    "mouseup",
    "mousemove",
    "focusin",
    "focusout",
  ];

  function defaultPrint(text) {
    console.log("📝 Volt:", text);
  }

  function defaultPrintErr(text) {
    console.error("❌ Volt Error:", text);
  }

  /**
   * Starts a Volt app using the global VoltApp factory.
   *
   * @param {Object} options
   * @param {string} [options.containerId="app-container"] - DOM element where events are listened from.
   * @param {string} [options.rootId="root"] - DOM element where VoltRuntime mounts (currently implicit).
   * @param {boolean} [options.debug=true] - Enable debug logging.
   * @param {string[]} [options.events] - List of DOM event types to forward to Volt.
   * @param {Object} [options.moduleOverrides] - Additional Emscripten module overrides.
   *
   * @returns {Promise<{ Module, voltNamespace, destroy: () => void }>}
   */
  function start(options) {
    const {
      containerId = "app-container",
      rootId = "root",
      debug = true,
      events = DEFAULT_EVENTS,
      moduleOverrides = {},
    } = options || {};

    if (typeof global.VoltApp !== "function") {
      throw new Error(
        "VoltApp factory not found. Make sure app.js (Emscripten output) is loaded before volt-app.js."
      );
    }

    const containerEl = document.getElementById(containerId);
    if (!containerEl) {
      throw new Error(
        `VoltBootstrap: container element with id="${containerId}" not found.`
      );
    }

    const rootEl = document.getElementById(rootId);
    if (!rootEl) {
      // Not fatal, but warn and continue (createRuntime currently uses hardcoded "root")
      console.warn(
        `VoltBootstrap: root element with id="${rootId}" not found. VoltRuntime will still create using its internal default.`
      );
    }

    const mergedModuleConfig = Object.assign(
      {
        print: debug ? defaultPrint : function () {},
        printErr: defaultPrintErr,
      },
      moduleOverrides
    );

    if (debug) {
      console.log("⚡ VoltBootstrap starting...");
      console.log("  containerId:", containerId);
      console.log("  rootId:", rootId);
      console.log("  events:", events.join(", "));
    }

    // Will hold handler reference for cleanup
    const eventHandlers = [];

    function attachEventHandlers(Module) {
      function makeHandler(eventType) {
        return function (event) {
          let target = event.target;
          const limit = this; // containerEl

          while (target && target !== limit) {
            if (target.__cpp_ptr) {
              try {
                Module.invokeBubbleEvent(target.__cpp_ptr, event);
              } catch (err) {
                console.error(
                  "❌ VoltBootstrap: error while invoking bubble event:",
                  err
                );
              }
              return;
            }
            target = target.parentNode;
          }
        };
      }

      events.forEach((type) => {
        const handler = makeHandler(type);
        containerEl.addEventListener(type, handler);
        eventHandlers.push({ type, handler });
      });

      if (debug) {
        console.log(
          `✅ VoltBootstrap: attached ${events.length} event listeners to #${containerId}`
        );
      }
    }

    function detachEventHandlers() {
      eventHandlers.forEach(({ type, handler }) => {
        containerEl.removeEventListener(type, handler);
      });
      eventHandlers.length = 0;
    }

    function showErrorOverlay(message) {
      try {
        const root = rootEl || containerEl;
        root.innerHTML =
          '<div style="color: red; padding: 20px;">Failed to load Volt app: ' +
          String(message) +
          "</div>";
      } catch (err) {
        console.error("VoltBootstrap: could not render error overlay:", err);
      }
    }

    return global
      .VoltApp(mergedModuleConfig)
      .then(function (Module) {
        if (debug) {
          console.log("🚀 Volt Runtime Initialized!");
        }

        let voltNamespace = null;
        if (typeof Module.getVoltNamespace === "function") {
          voltNamespace = Module.getVoltNamespace();
          if (debug) {
            console.log("🔧 Using namespace:", voltNamespace);
          }
        }

        attachEventHandlers(Module);

        if (typeof Module.createRuntime !== "function") {
          throw new Error(
            "Module.createRuntime() not found. Check your Emscripten/Volt bindings."
          );
        }

        // Currently createRuntime creates VoltRuntime internally.
        Module.createRuntime(rootId);

        if (debug) {
          console.log("✨ Volt app mounted!");
        }

        function destroy() {
          if (debug) {
            console.log("🧹 VoltBootstrap: destroying app instance...");
          }
          detachEventHandlers();
          // NOTE: If/when Volt exposes a runtime destroy/unmount API,
          // it should be called here.
        }

        return { Module, voltNamespace, destroy };
      })
      .catch(function (err) {
        console.error("❌ VoltBootstrap: failed to initialize Volt:", err);
        showErrorOverlay(err && err.message ? err.message : err);
        throw err;
      });
  }

  // Expose as a small global namespace
  global.VoltBootstrap = {
    start,
  };
})(window);
