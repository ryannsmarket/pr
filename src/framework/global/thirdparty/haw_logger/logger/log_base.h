#ifndef HAW_LOG_H
#define HAW_LOG_H

#include "helpful.h"
#include "logger.h"

#ifndef FUNC_INFO
#if defined(_MSC_VER)
    #define FUNC_INFO __FUNCSIG__
#else
    #define FUNC_INFO __PRETTY_FUNCTION__
#endif
#endif

//! Format
#define CLASSNAME(sig) haw::logger::Helpful::className(sig)
#define FUNCNAME(sig) haw::logger::Helpful::methodName(sig)

//! Log

#ifndef LOG_TAG
#define LOG_TAG CLASSNAME(FUNC_INFO)
#endif

#define IF_LOGLEVEL(level)  if (haw::logger::Logger::instance()->isLevel(level))

#define LOG_STREAM(type, tag, funcInfo, color) haw::logger::LogInput(type, tag, funcInfo, color).stream
#define LOG(type, tag, color)  LOG_STREAM(type, tag, FUNCNAME(FUNC_INFO) + ": ", color)

#define LOGE_T(tag, color) IF_LOGLEVEL(haw::logger::Normal) LOG(haw::logger::Logger::ERRR, tag, color)
#define LOGW_T(tag, color) IF_LOGLEVEL(haw::logger::Normal) LOG(haw::logger::Logger::WARN, tag, color)
#define LOGI_T(tag, color) IF_LOGLEVEL(haw::logger::Normal) LOG(haw::logger::Logger::INFO, tag, color)
#define LOGD_T(tag, color) IF_LOGLEVEL(haw::logger::Debug) LOG(haw::logger::Logger::DEBG, tag, color)

#define LOGE LOGE_T(LOG_TAG, haw::logger::Red)
#define LOGW LOGW_T(LOG_TAG, haw::logger::Yellow)
#define LOGI LOGI_T(LOG_TAG, haw::logger::Green)
#define LOGD LOGD_T(LOG_TAG, haw::logger::None)
#define LOGN if (0) LOGD_T(LOG_TAG, haw::logger::None) // compiling, but no output

//! Helps
#define DEPRECATED LOGD() << "This function deprecated!!"
#define DEPRECATED_USE(use) LOGD() << "This function deprecated!! Use:" << use
#define NOT_IMPLEMENTED LOGW() << "Not implemented!!"
#define NOT_IMPL_RETURN NOT_IMPLEMENTED; return
#define NOT_SUPPORTED LOGW() << "Not supported!!"
#define NOT_SUPPORTED_USE(use) LOGW() << "Not supported!! Use:" << use

#endif // HAW_LOG_H
