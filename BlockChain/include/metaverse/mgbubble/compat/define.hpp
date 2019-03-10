#pragma once

#ifndef MVS_ASH_DEFS_HPP
#define MVS_ASH_DEFS_HPP

/**
 * @addtogroup Util
 * @{
 */

 /**
  * Macro for exporting classes and functions that compose the public API.
  */
#ifdef _WIN32
#define MVS_API
#else
#define MVS_API __attribute__((visibility("default"))) //设置导出符号表，"hidden"就会隐藏
#endif

#define LOG_HTTP "http"

  /** @} */

#endif // MVS_ASH_DEFS_HPP