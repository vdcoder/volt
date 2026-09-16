// Seasocks unity translation unit. Compile this file, not its included .cpp files.
// wepoll.c stays a separate C translation unit.

#include "../dependencies/seasocks/generated/Embedded.cpp"
#include "../dependencies/seasocks/src/main/c/AcceptEncoding.cpp"
#include "../dependencies/seasocks/src/main/c/Connection.cpp"
#include "../dependencies/seasocks/src/main/c/HybiAccept.cpp"
#include "../dependencies/seasocks/src/main/c/HybiPacketDecoder.cpp"
// The Windows compatibility header defines access as _access. Keep that
// macro from rewriting the unrelated Logger::access method definition.
#pragma push_macro("access")
#undef access
#include "../dependencies/seasocks/src/main/c/Logger.cpp"
#pragma pop_macro("access")
#include "../dependencies/seasocks/src/main/c/PageRequest.cpp"
#include "../dependencies/seasocks/src/main/c/Response.cpp"
#include "../dependencies/seasocks/src/main/c/Server.cpp"
#include "../dependencies/seasocks/src/main/c/StringUtil.cpp"
#include "../dependencies/seasocks/src/main/c/internal/Base64.cpp"
#include "../dependencies/seasocks/src/main/c/md5/md5.cpp"
#include "../dependencies/seasocks/src/main/c/sha1/sha1.cpp"
#include "../dependencies/seasocks/src/main/c/seasocks/Request.cpp"
#include "../dependencies/seasocks/src/main/c/seasocks/ResponseBuilder.cpp"
#include "../dependencies/seasocks/src/main/c/seasocks/ResponseCode.cpp"
#include "../dependencies/seasocks/src/main/c/seasocks/StreamingResponse.cpp"
#include "../dependencies/seasocks/src/main/c/seasocks/SynchronousResponse.cpp"
#include "../dependencies/seasocks/src/main/c/seasocks/ZlibContextDisabled.cpp"
#include "../dependencies/seasocks/src/main/c/util/CrackedUri.cpp"
#include "../dependencies/seasocks/src/main/c/util/Json.cpp"
#include "../dependencies/seasocks/src/main/c/util/PathHandler.cpp"
#include "../dependencies/seasocks/src/main/c/util/RootPageHandler.cpp"
