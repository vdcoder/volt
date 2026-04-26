// ============================================================================
// test/ultralight – Ultralight SDK Headless Smoke Test
//
// Minimal self-contained test. Implements only the bare minimum Platform
// handlers needed to load inline HTML, render it, and evaluate JavaScript.
//
// Links directly against the SDK binaries (UltralightCore, WebCore, Ultralight).
// ============================================================================

#include <Ultralight/Ultralight.h>
#include <Ultralight/platform/Platform.h>
#include <Ultralight/platform/Config.h>
#include <Ultralight/platform/Logger.h>
#include <Ultralight/platform/FileSystem.h>
#include <Ultralight/platform/FontLoader.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <fstream>
#include <vector>
#include <Windows.h>

using namespace ultralight;

// ===========================================================================
// Minimal Logger – writes everything to stdout
// ===========================================================================
class StderrLogger : public Logger {
  void LogMessage(LogLevel level, const String& message) override {
    const char* tag = "";
    switch (level) {
      case LogLevel::Error:   tag = "[UL-ERROR] "; break;
      case LogLevel::Warning: tag = "[UL-WARN]  "; break;
      case LogLevel::Info:    tag = "[UL-INFO]  "; break;
    }
    std::cout << tag << message.utf8().data() << std::endl;
  }
};

// ===========================================================================
// Minimal FileSystem – reads files from disk using standard library
// ===========================================================================
class StubFileSystem : public FileSystem {
  bool FileExists(const String& path) override {
    auto u8 = path.utf8();
    std::ifstream f(u8.data());
    bool ok = f.good();
    f.close();
    return ok;
  }
  String GetFileMimeType(const String& path) override {
    auto u8 = path.utf8();
    auto s = u8.data();
    const char* ext = strrchr(s, '.');
    if (!ext) return String();
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return String("text/html");
    if (strcmp(ext, ".css") == 0) return String("text/css");
    if (strcmp(ext, ".js") == 0) return String("application/javascript");
    if (strcmp(ext, ".png") == 0) return String("image/png");
    if (strcmp(ext, ".dat") == 0) return String("application/octet-stream");
    if (strcmp(ext, ".pem") == 0) return String("application/x-pem-file");
    return String("application/octet-stream");
  }
  String GetFileCharset(const String&) override { return String(); }
  RefPtr<Buffer> OpenFile(const String& path) override {
    auto u8 = path.utf8();
    std::ifstream f(u8.data(), std::ios::binary | std::ios::ate);
    if (!f) return nullptr;
    auto sz = (size_t)f.tellg();
    f.seekg(0);
    std::vector<char> data(sz);
    f.read(data.data(), sz);
    return Buffer::CreateFromCopy(data.data(), sz);
  }
};

// ===========================================================================
// FontLoader – DirectWrite-based, loads system fonts
// ===========================================================================
#include <dwrite.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

class WinFontLoader : public FontLoader {
  ComPtr<IDWriteFactory> factory_;
  ComPtr<IDWriteFontCollection> sysCollection_;

public:
  WinFontLoader() {
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                        &factory_);
    if (factory_)
      factory_->GetSystemFontCollection(&sysCollection_);
  }

  String fallback_font() const override {
    return String("Arial");
  }

  String fallback_font_for_characters(const String&, int, bool) const override {
    return String("Arial");
  }

  RefPtr<FontFile> Load(const String& family, int weight, bool italic) override {
    auto u8 = family.utf8();
    if (!sysCollection_ || !u8.data() || !u8.data()[0])
      return nullptr;

    // Convert to wchar for DirectWrite
    int wlen = MultiByteToWideChar(CP_UTF8, 0, u8.data(), -1, nullptr, 0);
    if (wlen <= 0) return nullptr;
    std::vector<wchar_t> wfam(wlen);
    MultiByteToWideChar(CP_UTF8, 0, u8.data(), -1, wfam.data(), wlen);

    // Find family in system collection
    UINT32 idx = 0;
    BOOL exists = FALSE;
    sysCollection_->FindFamilyName(wfam.data(), &idx, &exists);
    if (!exists) return nullptr;

    // Get the first font in this family matching desired weight/italic
    ComPtr<IDWriteFontFamily> font;
    sysCollection_->GetFontFamily(idx, &font);
    if (!font) return nullptr;

    for (UINT32 fi = 0; fi < font->GetFontCount(); ++fi) {
      ComPtr<IDWriteFont> dwFont;
      if (FAILED(font->GetFont(fi, &dwFont))) continue;

      auto dwWeight = dwFont->GetWeight();
      auto dwStyle  = dwFont->GetStyle();

      // We'll accept approximate weight match (within 100) and exact italic
      LONG realW = (weight < 0) ? 400 : weight;
      bool wMatch = abs((int)dwWeight - realW) <= 100;
      bool iMatch = (italic && dwStyle == DWRITE_FONT_STYLE_ITALIC) ||
                    (!italic && dwStyle == DWRITE_FONT_STYLE_NORMAL);
      if (!wMatch || !iMatch) continue;

      // Get font face
      ComPtr<IDWriteFontFace> face;
      if (FAILED(dwFont->CreateFontFace(&face))) continue;

      // Get font file reference
      UINT32 fileCount = 0;
      if (FAILED(face->GetFiles(&fileCount, nullptr)) || fileCount == 0) continue;
      ComPtr<IDWriteFontFile> file;
      { IDWriteFontFile* fp = nullptr;
        UINT32 c = 1;
        if (FAILED(face->GetFiles(&c, &fp))) continue;
        file.Attach(fp);
      }

      // Get reference key + loader to read the actual font data
      void const* key = nullptr;
      UINT32 keySize = 0;
      file->GetReferenceKey(&key, &keySize);
      ComPtr<IDWriteFontFileLoader> loader;
      if (FAILED(file->GetLoader(&loader))) continue;
      if (!loader) continue;

      ComPtr<IDWriteFontFileStream> stream;
      if (FAILED(loader->CreateStreamFromKey(key, keySize, &stream)))
        continue;

      UINT64 fileSize = 0;
      if (FAILED(stream->GetFileSize(&fileSize))) continue;

      void const* fragBegin = nullptr;
      void* fragCtx = nullptr;
      if (FAILED(stream->ReadFileFragment(&fragBegin, 0, fileSize, &fragCtx)))
        continue;

      auto buf = Buffer::CreateFromCopy(fragBegin, (size_t)fileSize);
      stream->ReleaseFileFragment(fragCtx);

      if (buf)
        return FontFile::Create(buf);
    }

    return nullptr;
  }
};

// ===========================================================================
// LoadListener – signals page ready
// ===========================================================================
class DoneFlagListener : public LoadListener {
  bool& done_;
public:
  DoneFlagListener(bool& f) : done_(f) {}
  void OnFinishLoading(ultralight::View*, uint64_t, bool, const String&) override {
    done_ = true;
    std::cout << "[test] ✅ OnFinishLoading fired" << std::endl;
  }
  void OnFailLoading(ultralight::View*, uint64_t, bool, const String&,
                     const String& desc, const String&, int code) override {
    std::cout << "[test] ❌ OnFailLoading: " << desc.utf8().data()
              << " (code=" << code << ")" << std::endl;
    done_ = true; // don't hang
  }
};

// ===========================================================================
// Test HTML – kept short. Uses safe raw-string delimiter.
// ===========================================================================
const char* kTestHTML = R"html(
<!DOCTYPE html>
<html>
<head>
<style>
  body  { background:#1a1a2e; color:#00d4aa; font-family:sans-serif;
          text-align:center; padding:40px; margin:0; }
  h1    { font-size:2em; margin-bottom:0.2em; }
  .num  { color:#e94560; font-size:3em; font-weight:bold; }
  .msg  { color:#e0e0e0; margin-top:1em; font-family:monospace; }
</style>
</head>
<body>
  <h1>&#x26A1; Volt &#x00D7; Ultralight</h1>
  <p>Count: <span id="counter" class="num">0</span></p>
  <p id="status" class="msg">Waiting...</p>
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
  std::cout << "[test] === Ultralight Headless Smoke Test ===" << std::endl;

  // ---- 1. Platform setup ----
  auto& plat = Platform::instance();

  Config cfg;
  cfg.scroll_timer_delay = 0.0;
  plat.set_config(cfg);

  StderrLogger log;
  plat.set_logger(&log);

  StubFileSystem fs;
  plat.set_file_system(&fs);

  WinFontLoader fl;
  plat.set_font_loader(&fl);

  // ---- 2. Renderer ----
  std::cout << "[test] Creating Renderer..." << std::endl;
  auto renderer = Renderer::Create();

  // ---- 3. View ----
  std::cout << "[test] Creating View 800x600 (CPU)..." << std::endl;
  ViewConfig vc;
  vc.is_accelerated = false;
  auto view = renderer->CreateView(800, 600, vc, nullptr);
  if (!view) {
    std::cout << "[test] ❌ CreateView returned null!" << std::endl;
    return 1;
  }
  std::cout << "[test] ✅ View created: " << view->width() << "x" << view->height() << std::endl;

  // ---- 4. Load listener ----
  bool loaded = false;
  DoneFlagListener dl(loaded);
  view->set_load_listener(&dl);

  // ---- 5. Load inline HTML ----
  std::cout << "[test] LoadHTML..." << std::endl;
  view->LoadHTML(kTestHTML, "file:///test.html");

  // ---- 6. Pump until page loaded or timeout ----
  auto t0 = std::chrono::steady_clock::now();
  while (!loaded) {
    renderer->Update();
    renderer->Render();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto dt = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - t0).count();
    if (dt > 10.0) {
      std::cout << "[test] ⚠️  Load timeout (10 s)" << std::endl;
      break;
    }
  }

  // ---- 7. JS interop via EvaluateScript ----
  std::cout << "\n[test] --- EvaluateScript ---" << std::endl;

  // Check status field
  String s = view->EvaluateScript(
    "document.getElementById('status').textContent");
  std::cout << "  status = \"" << s.utf8().data() << "\"" << std::endl;

  // Increment 3 times
  view->EvaluateScript("inc(); inc(); inc();");
  String c = view->EvaluateScript(
    "document.getElementById('counter').textContent");
  std::cout << "  count  = \"" << c.utf8().data() << "\"" << std::endl;

  bool jsOK = (strcmp(c.utf8().data(), "3") == 0);
  std::cout << (jsOK ? "  ✅ JS eval: PASS" : "  ❌ JS eval: FAIL") << std::endl;

  // ---- 8. Bitmap surface ----
  std::cout << "\n[test] --- Surface ---" << std::endl;
  Surface* surf = view->surface();
  if (!surf) {
    std::cout << "  ❌ No surface" << std::endl;
    return 1;
  }
  RefPtr<Bitmap> bmp = ((BitmapSurface*)surf)->bitmap();
  if (!bmp) {
    std::cout << "  ❌ No bitmap" << std::endl;
    return 1;
  }

  uint32_t w = bmp->width(), h = bmp->height();
  std::cout << "  Bitmap: " << w << " x " << h << std::endl;

  uint8_t* px = (uint8_t*)bmp->LockPixels();
  if (px) {
    // Top-left pixel — should be dark navy (#1a1a2e)  BGRA
    int r0 = (int)px[2], g0 = (int)px[1], b0 = (int)px[0];
    std::cout << "  Pixel(0,0) BGRA: r=" << r0 << " g=" << g0 << " b=" << b0
              << std::endl;

    // Center pixel
    size_t off = (h/2) * w * 4 + (w/2) * 4;
    if (off + 3 < (size_t)w * h * 4) {
      int rc = (int)px[off+2], gc = (int)px[off+1], bc = (int)px[off+0];
      std::cout << "  Pixel(c)  BGRA: r=" << rc << " g=" << gc << " b=" << bc
                << std::endl;
    }

    bool hasPixels = (b0 > 5 || g0 > 5 || r0 > 5); // not all zero
    std::cout << (hasPixels ? "  ✅ Bitmap populated: PASS"
                            : "  ⚠️  Bitmap seems empty") << std::endl;
  }
  bmp->UnlockPixels();

  // ---- Summary ----
  bool allOK = jsOK;
  std::cout << "\n[test] =================================" << std::endl;
  std::cout << "[test] " << (allOK ? "🎉 ALL TESTS PASSED" : "❌ SOME TESTS FAILED")
            << std::endl;
  std::cout << "[test] =================================" << std::endl;
  return allOK ? 0 : 1;
}
