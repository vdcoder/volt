# Seasocks snapshot

Source: https://github.com/mattgodbolt/seasocks
Commit: 8d56aaab70227860cc9318ffa0dd03e34065f00c (version 1.4.6)
License: BSD-2-Clause; see LICENSE and notices in source files.

The src/main and scripts directories are copied from this revision. Server
compiles the C++ library sources through server/seasocks_impl.cpp and the
Windows wepoll implementation as a separate C translation unit through MSBuild.
The unity wrapper temporarily undefines the Windows `access` macro around
Logger.cpp to preserve the Logger::access method name. No CMake, dependency
download, or Python invocation is needed to build.
Deflate is disabled using upstream ZlibContextDisabled.cpp; zlib is not required.

Local modifications in Connection.cpp:
- Check WSAEWOULDBLOCK on Windows when send buffers fill, allowing large WASM
  responses to resume instead of incorrectly closing the socket.
- Added `wasm` -> `application/wasm` to the MIME table for streaming compilation.
- Reply with an empty WebSocket Close frame before closing TCP on a peer Close;
  upstream immediately shut down TCP, causing modern clients to report an error.

generated/internal/Config.h corresponds to upstream Config.h.in
with version 1.4.6 and deflateEnabled=false. generated/Embedded.cpp was generated
with scripts/gen_embedded.py from the seven files listed in src/main/web/CMakeLists.txt.
Embedded assets retain their upstream notices, including jQuery's license.
