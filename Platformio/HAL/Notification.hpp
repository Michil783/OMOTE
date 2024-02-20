#pragma once
#include <functional>
#include <map>
#include <memory>

template <class... notifyData> class Handler;
template <class... notifyData> class Notification {
  friend class Handler<notifyData...>;

public:
  typedef std::function<void(notifyData...)> HandlerTy;
  typedef int HandlerID;

  Notification() { mIdMaker = 0; };
  void notify(notifyData... notifySendData);

protected:
  HandlerID onNotify(HandlerTy aHandler);
  void unregister(HandlerID aHandler);

private:
  std::map<HandlerID, HandlerTy> mFunctionHandlers;
  HandlerID mIdMaker;
};

template <class... handlerData>
int Notification<handlerData...>::onNotify(HandlerTy aHandler) {
  printf(">>> Notification::onNotify(%s)\n", typeid(aHandler).name());
  if (aHandler) {
    mFunctionHandlers[++mIdMaker] = std::move(aHandler);
    printf("<<< Notification::onNotify() success %d\n", mIdMaker);
    printf("number of mFunctionHandlers: %ld\n", mFunctionHandlers.size()); 
   return mIdMaker;
  } else {
    printf("<<< Notification::onNotify() failed\n");
    return -1;
  }
}

template <class... outboundData>
void Notification<outboundData...>::notify(outboundData... notifySendData) {
  printf(">>> Notification::notify()\n");
  printf("type of notifySendData: %s\n", typeid(notifySendData).name()...);
  printf("number of handlers: %ld\n", mFunctionHandlers.size());
  for (auto handler : mFunctionHandlers) {
    printf("calling handler.second(notifySendData...)\n");
    handler.second(notifySendData...);
  }
  printf("<<< Notification::notify()\n");
}

template <class... handlerData>
void Notification<handlerData...>::unregister(HandlerID aHandlerId) {
  printf(">>> Notification::unregister(%d)\n", aHandlerId);
  auto handlerToUnRegister =
      std::find_if(mFunctionHandlers.begin(), mFunctionHandlers.end(),
                   [aHandlerId](auto registeredHandler) {
                     return aHandlerId == registeredHandler.first;
                   });
  if (handlerToUnRegister != mFunctionHandlers.end()) {
    mFunctionHandlers.erase(handlerToUnRegister);
  }
  printf("<<< Notification::unregister(%d)\n", aHandlerId);
}

template <class... notifyData> class Handler {
public:
  typedef std::function<void(notifyData...)> callableTy;
  void operator=(Handler &other) = delete;

  Handler() = default;
  Handler(std::shared_ptr<Notification<notifyData...>> aNotification,
          callableTy aCallable = nullptr)
      : mNotification(aNotification),
        mHandlerId(aNotification->onNotify(aCallable)) { 
          printf(">>> Notification::Handler()\n");
          printf("aNotification: %s\n", typeid(aNotification).name()); 
          printf("aCallable: %s\n", typeid(aCallable).name());
          printf("<<< Notification::Handler()\n");
          }

  virtual ~Handler() {
    printf(">>> Notification::~Handler()\n");
    printf("mHandlerId: %d\n", mHandlerId);
    if (mHandlerId >= 0) {
      mNotification->unregister(mHandlerId);
    }
    printf("<<< Notification::~Handler()\n");
  }

  void operator=(callableTy aHandler) {
    if (mHandlerId >= 0) {
      printf("operator= unregister mHandlerId %d\n", mHandlerId);
      mNotification->unregister(mHandlerId);
      mHandlerId = -1;
    }
    if (aHandler) {
      mHandlerId = mNotification->onNotify(aHandler);
    }
  }

  void
  SetNotification(std::shared_ptr<Notification<notifyData...>> aNotification) {
    mNotification = aNotification;
  }

private:
  std::shared_ptr<Notification<notifyData...>> mNotification = nullptr;
  int mHandlerId = -1;
};
