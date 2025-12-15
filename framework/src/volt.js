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
    "abort",
    "afterprint",
    "beforeprint",
    "beforeunload",
    "blur",
    "canplay",
    "canplaythrough",
    "change",
    "click",
    "contextmenu",
    "copy",
    "cuechange",
    "cut",
    "dblclick",
    "drag",
    "dragend",
    "dragenter",
    "dragleave",
    "dragover",
    "dragstart",
    "drop",
    "durationchange",
    "emptied",
    "ended",
    "error",
    "focus",
    "focusin",
    "focusout",
    "hashchange",
    "input",
    "invalid",
    "keydown",
    "keypress",
    "keyup",
    "load",
    "loadeddata",
    "loadedmetadata",
    "loadstart",
    "mousedown",
    "mousemove",
    "mouseout",
    "mouseover",
    "mouseup",
    "mousewheel",
    "offline",
    "online",
    "pagehide",
    "pageshow",
    "paste",
    "pause",
    "play",
    "playing",
    "pointerover",
    "pointerdown",
    "pointermove",
    "pointerup",
    "pointercancel",
    "pointerout",
    "pointerrawupdate",
    "popstate",
    "progress",
    "ratechange",
    "reset",
    "resize",
    "scroll",
    "search",
    "seeked",
    "seeking",
    "select",
    "stalled",
    "storage",
    "submit",
    "suspend",
    "timeupdate",
    "toggle",
    "touchstart",
    "touchmove",
    "unload",
    "volumechange",
    "waiting",
    "wheel",
  ];

  const defaultPassiveEventNames = new Set([
    "touchstart",
    "touchmove",
    "wheel",
  ]);

  function isPassiveEvent(passiveEvents, eventName) {
    return passiveEvents.has(eventName);
  }

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
      passiveEvents = defaultPassiveEventNames,
    } = options || {};

    if (typeof global.VoltApp !== "function") {
      throw new Error(
        "VoltApp factory not found. Make sure app.js (Emscripten output) is loaded before volt.js."
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
      throw new Error(`VoltBootstrap: root element with id="${rootId}" not found.`);
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
      const genericHandler = (event) => {
        let target = event.target;
        while (target && target !== containerEl) {
          if (target.__cpp_ptr) {
            // Sanity check (temporary)
            const t = typeof target.__cpp_ptr;
            if (t !== "bigint" && t !== "number") {
              console.warn("VoltBootstrap: unexpected __cpp_ptr type:", t, target.__cpp_ptr);
            }
            
            event.__volt_cpp_ptr = target.__cpp_ptr;
            try {
              Module.invokeVoltBubbleEvent(event);
            } catch (err) {
              console.error("❌ VoltBootstrap: error while invoking bubble event:", err);
            }
            if (event.cancelBubble === true) {
              break;
            }
          }
          target = target.parentNode;
        }
      };

      function makeFocusInOutHandlers() {
        /** @type {{ type: 'in'|'out', event: FocusEvent }[]} */
        const pending = [];
        let processing = false;

        function processFocusIn(event) {
          // We’re entering / changing focus within this container
          Module.clearVoltFocussedElements();

          // Focus-register every node up to the container
          let target = event.target;
          while (target && target !== containerEl) {
            try {
              if (target.__cpp_ptr) {
                Module.addVoltFocussedElement(target);
              }
            } catch (err) {
              console.error("❌ VoltBootstrap: error in focusin focus-register handler:", err);
            }
            target = target.parentNode;
          }

          // Call bubble event handlers up to the container
          target = event.target;
          while (target && target !== containerEl) {
            if (target.__cpp_ptr) {
              event.__volt_cpp_ptr = target.__cpp_ptr;
              try {
                Module.invokeVoltBubbleEvent(event);
              } catch (err) {
                console.error("❌ VoltBootstrap: error in focusin handler:", err);
              }
              if (event.cancelBubble === true) {
                break;
              }
            }
            target = target.parentNode;
          }
        }

        function processFocusOut(event) {
          const related = event.relatedTarget;

          if (related && containerEl.contains(related)) {
            return;
          }

          // Focus has left the container entirely (or to null / browser chrome / other region).
          try {
            Module.clearVoltFocussedElements();
          } catch (err) {
            console.error("❌ VoltBootstrap: error in focusout handler:", err);
          }
        }

        function drain() {
          if (processing) return;
          processing = true;
          try {
            while (pending.length > 0) {
              const { type, event } = pending.shift();
              if (type === "in") {
                processFocusIn(event);
              } else {
                processFocusOut(event);
              }
            }
          } finally {
            processing = false;
          }
        }

        function focusInHandler(event) {
          pending.push({ type: "in", event });
          drain();
        }

        function focusOutHandler(event) {
          pending.push({ type: "out", event });
          drain();
        }

        return { focusInHandler, focusOutHandler };
      }

      const { focusInHandler, focusOutHandler } = makeFocusInOutHandlers();
      events.forEach((type) => {
        const handler = type === "focusin" ? focusInHandler : type === "focusout" ? focusOutHandler : genericHandler;
        containerEl.addEventListener(type, handler, { passive: isPassiveEvent(passiveEvents, type) });
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

        if (typeof Module.createVoltEngine !== "function") {
          throw new Error(
            "Module.createVoltEngine() not found. Check your Emscripten/Volt bindings."
          );
        }

        // Currently createVoltEngine creates VoltRuntime internally.
        Module.createVoltEngine(rootId);

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


// ==================================================
//   VOLT DEBUG CONSOLE (Global: window.volt)
// ==================================================
(function() {

    const VOLT_PREFIX = "Volt>";
    const GLOBAL_NAME = "volt";

    // Holds enabled categories
    let enabled = new Set();

    // Holds minimum global level
    let globalLevel = 0; // TRACE_L

    // Friendly level map (mirrors C++ enum values)
    const levels = {
        trace: 0,
        debug: 1,
        info:  2,
        warn:  3,
        error: 4,
    };

    const levelNames = ["TRACE","DEBUG","INFO","WARN","ERROR"];

    // Pretty formatting
    function colorForLevel(lvl) {
        switch (lvl) {
            case 0: return "color:#888";
            case 1: return "color:#55f";
            case 2: return "color:#0a0";
            case 3: return "color:#da0";
            case 4: return "color:#f33";
            default: return "";
        }
    }

    const STORAGE_KEY = "volt.log.settings";

    function loadState() {
        try {
            if (typeof window === "undefined" || !window.localStorage) return;

            const raw = window.localStorage.getItem(STORAGE_KEY);
            if (!raw) return;

            const parsed = JSON.parse(raw);
            if (parsed && Array.isArray(parsed.enabled) && typeof parsed.level === "number") {
                enabled = new Set(parsed.enabled);
                globalLevel = parsed.level;
                // Optional: announce in console
                console.log(
                    "Volt logging restored from storage:",
                    {
                        enabled: [...enabled],
                        level: levelNames[globalLevel] || globalLevel
                    }
                );
            }
        } catch (e) {
            console.warn("Volt: failed to restore log settings:", e);
        }
    }

    function saveState() {
        try {
            if (typeof window === "undefined" || !window.localStorage) return;

            const payload = {
                enabled: [...enabled],
                level: globalLevel,
            };
            window.localStorage.setItem(STORAGE_KEY, JSON.stringify(payload));
        } catch (e) {
            console.warn("Volt: failed to persist log settings:", e);
        }
    }

    // Load persisted settings once on init
    loadState();

    // Public API object
    const api = {
        /**
         * Enable a category prefix. Example:
         * volt.on("DiffPatch")
         */
        on(cat = null) {
            if (!cat) return api.help(); // print help if no args

            enabled.add(cat);
            saveState();
            console.log(`✔ Volt logging enabled for category: ${cat}`);
        },

        /**
         * Disable a category or reset all categories if none specified.
         */
        off(cat = null) {
            if (cat === null) {
                // Full reset
                enabled.clear();
                globalLevel = levels.info;   // optional: reset level to default
                saveState();
                console.log("✔ Volt logging reset. All categories disabled.");
                console.log("  Level = INFO");
                return;
            }

            // Disable just one
            enabled.delete(cat);
            saveState();
            console.log(`✔ Volt logging disabled for: ${cat}`);
        },

        /**
         * Show current enabled categories
         */
        show() {
            if (enabled.size === 0)
                return console.log("No Volt log categories enabled.");

            console.log("Enabled Volt categories:");
            for (const c of enabled) console.log(" • " + c);
        },

        /**
         * Set global level threshold: "trace","debug","info","warn","error"
         */
        level(lvl) {
            lvl = lvl.toLowerCase();
            if (!(lvl in levels)) {
                console.log(`Unknown level '${lvl}'. Valid: trace, debug, info, warn, error.`);
                return;
            }
            globalLevel = levels[lvl];
            saveState();
            console.log(`✔ Volt log level = ${lvl.toUpperCase()}`);
        },

        /**
         * Print all commands
         */
        help() {
            console.log(
`==================== Volt Debug Console ====================

volt.on("<category>")     Enable a category (prefix match)
volt.off("<category>")    Disable category
volt.show()               Show all enabled categories
volt.level("<level>")     Set minimum level (trace,debug,info,warn,error)
volt.help()               Show this help

Current:
  level     = ${levelNames[globalLevel] || globalLevel}
  enabled   = ${enabled.size ? [...enabled].join(", ") : "<none>"}

Example:
  volt.on("Volt>DiffPatch")
  volt.level("debug")

=============================================================`
            );
        },

        // INTERNAL: used by C++ logging bridge
        _shouldPrint(level, category) {
            if (level < globalLevel) return false;
            if (enabled.size === 0) return false;
            for (const prefix of enabled) {
                if (category.startsWith(prefix)) return true;
            }
            return false;
        },

        // INTERNAL: used by C++ logging bridge
        _print(level, category, indent, message) {
            if (!api._shouldPrint(level, category)) return;

            const lvlName = levelNames[level] || "LOG";
            const padding = " ".repeat(indent * 2);

            console.log(
                `%c${lvlName}%c ${category}%c ${padding}${message}`,
                colorForLevel(level),
                "color:#0af;font-weight:bold",
                "color:inherit"
            );
        },
    };

    // Expose globally
    window[GLOBAL_NAME] = api;

})();
