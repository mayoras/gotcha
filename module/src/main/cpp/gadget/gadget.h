#ifndef GOTCHA_GADGET_H
#define GOTCHA_GADGET_H

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#if GOTCHA_DEBUG
#define LOG_TAG "[GotchaGadgetDebug]"
#else
#define LOG_TAG "[GotchaGadget]"
#endif

#endif //GOTCHA_GADGET_H
