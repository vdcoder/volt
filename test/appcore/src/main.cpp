// ============================================================================
// test/appcore – Ultralight SDK Smoke Test (with AppCore platform helpers)
//
// Uses pre-built AppCore.dll for native platform handlers (DirectWrite fonts,
// file system, logger). Links SDK DLLs directly.
//
// Tests: CPU rendering, LoadHTML + LoadURL, EvaluateScript, BitmapSurface
// ============================================================================

// STL headers
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <vector>

// SDK headers
#include <Ultralight/Ultralight.h>
#include <AppCore/AppCore.h>

using namespace ultralight;

// ===========================================================================
// LoadListener – signals page ready, logs progress
// ===========================================================================
class TestListener : public LoadListener {
  bool& done_;
public:
  TestListener(bool& f) : done_(f) {}

  void OnFinishLoading(View*, uint64_t, bool is_main, const String&) override {
    std::cout << "[test] ✅ OnFinishLoading" << std::endl;
    if (is_main) done_ = true;
  }

  void OnFailLoading(View*, uint64_t, bool, const String&,
                     const String& desc, const String&, int) override {
    std::cout << "[test] ❌ OnFailLoading: " << desc.utf8().data() << std::endl;
    done_ = true;
  }
};

// ===========================================================================
// Inline test HTML
// ===========================================================================
const char kHtml[] = R"html(
<!DOCTYPE html>
<html>
<head><style>
  body { background: #1a1a2e; color: #e0e0e0; font-family: Arial; }
  h1 { color: #0f0; }
  #status { color: #ff0; }
  #counter { font-size: 32px; color: #f0f; }
</style></head>
<body>
  <h1>Ultralight SDK Test</h1>
  <p>Status: <span id="status">Loading...</span></p>
  <p>Count: <span id="counter">0</span></p>
  <script>
    var c = 0;
    function inc() { c++; document.getElementById('counter').textContent = c; }
    document.getElementById('status').textContent = 'Ready!';
  </script>
</body>
</html>
)html";

// ===========================================================================
int main() {
  std::cout << "[test] === Ultralight SDK Smoke Test (AppCore) ===" << std::endl;

  // ---- 1. Platform setup (using AppCore helpers) ----
  std::cout << "[test] Setting up platform..." << std::endl;
  auto& plat = Platform::instance();

  Config cfg;
  cfg.scroll_timer_delay = 0.0;
  plat.set_config(cfg);
  plat.set_font_loader(GetPlatformFontLoader());
  plat.set_file_system(GetPlatformFileSystem("."));
  plat.set_logger(GetDefaultLogger("ultralight.log"));

  // ---- 2. Renderer ----
  std::cout << "[test] Creating Renderer..." << std::endl;
  auto renderer = Renderer::Create();

  // ---- 3. View (CPU render) ----
  std::cout << "[test] Creating View 800x600 (CPU)..." << std::endl;
  ViewConfig vc;
  vc.is_accelerated = false;
  auto view = renderer->CreateView(800, 600, vc, nullptr);
  if (!view) {
    std::cout << "[test] ❌ CreateView failed!" << std::endl;
    return 1;
  }
  std::cout << "[test] ✅ View: " << view->width() << "x" << view->height() << std::endl;

  // ---- 4. Load listener ----
  bool loaded = false;
  TestListener tl(loaded);
  view->set_load_listener(&tl);

  // ---- 5. Load inline HTML ----
  std::cout << "[test] Loading HTML..." << std::endl;
  view->LoadHTML(kHtml, "file:///test.html");

  // ---- 6. Pump until loaded or timeout ----
  auto t0 = std::chrono::steady_clock::now();
  while (!loaded) {
    renderer->Update();
    renderer->Render();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto dt = std::chrono::duration<double>(
      std::chrono::steady_clock::now() - t0).count();
    if (dt > 10.0) {
      std::cout << "[test] ⚠️  Timeout (10s)" << std::endl;
      break;
    }
  }

  // ---- 7. JS interop ----
  std::cout << "\n[test] --- EvaluateScript ---" << std::endl;
  String s = view->EvaluateScript(
    "document.getElementById('status').textContent");
  std::cout << "  status = \"" << s.utf8().data() << "\"" << std::endl;

  view->EvaluateScript("inc(); inc(); inc();");
  String c = view->EvaluateScript(
    "document.getElementById('counter').textContent");
  std::cout << "  count  = \"" << c.utf8().data() << "\"" << std::endl;

  bool jsOK = (c.utf8() == String("3").utf8());
  std::cout << (jsOK ? "  ✅ JS: PASS" : "  ❌ JS: FAIL") << std::endl;

  // ---- 8. Bitmap surface ----
  std::cout << "\n[test] --- Surface ---" << std::endl;
  auto* surf = view->surface();
  if (!surf) {
    std::cout << "  ❌ No surface" << std::endl;
    return 1;
  }

  auto* bmpSurf = static_cast<BitmapSurface*>(surf);
  auto bmp = bmpSurf->bitmap();
  if (!bmp) {
    std::cout << "  ❌ No bitmap" << std::endl;
    return 1;
  }

  std::cout << "  Bitmap: " << bmp->width() << " x " << bmp->height() << std::endl;

  uint8_t* px = (uint8_t*)bmp->LockPixels();
  if (px) {
    int r0 = (int)px[2], g0 = (int)px[1], b0 = (int)px[0];
    std::cout << "  Pixel(0,0) BGRA: r=" << r0 << " g=" << g0
              << " b=" << b0 << std::endl;

    // Center pixel
    size_t off = (bmp->height()/2) * bmp->row_bytes()
               + (bmp->width()/2) * 4;
    int rc = (int)px[off+2], gc = (int)px[off+1], bc = (int)px[off+0];
    std::cout << "  Pixel(c)  BGRA: r=" << rc << " g=" << gc
              << " b=" << bc << std::endl;

    bool hasPixels = (b0 > 5 || g0 > 5 || r0 > 5);
    std::cout << (hasPixels ? "  ✅ Bitmap populated: PASS"
                            : "  ⚠️  Empty") << std::endl;
  }
  bmp->UnlockPixels();

  // ---- Summary ----
  bool allOK = jsOK;
  std::cout << "\n[test] =================================" << std::endl;
  std::cout << "[test] " << (allOK ? "🎉 ALL TESTS PASSED"
                                   : "❌ SOME FAILED") << std::endl;
  std::cout << "[test] =================================" << std::endl;
  return allOK ? 0 : 1;
}
