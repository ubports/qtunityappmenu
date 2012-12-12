// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "integration.h"
#include "window.h"
#include "input.h"
#include "base/logging.h"
#include <QtCore/QCoreApplication>
#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatforminputcontextfactory_p.h>
#include <qpa/qplatforminputcontext.h>

static void resumedCallback(void* context) {
  DLOG("resumedCallback (context=%p)", context);
  DASSERT(context != NULL);
  // FIXME(loicm) Add support for resumed callback.
  // QHybrisScreen* screen = static_cast<QHybrisScreen*>(context);
}

static void suspendedCallback(void* context) {
  DLOG("suspendedCallback (context=%p)", context);
  DASSERT(context != NULL);
  // FIXME(loicm) Add support for suspended callback.
  // QHybrisScreen* screen = static_cast<QHybrisScreen*>(context);
}

static void focusedCallback(void* context) {
  DLOG("focusedCallback (context=%p)", context);
  DASSERT(context != NULL);
  // FIXME(loicm) Add support for focused callback.
  // QHybrisScreen* screen = static_cast<QHybrisScreen*>(context);
}

static void unfocusedCallback(void* context) {
  DLOG("unfocusedCallback (context=%p)", context);
  DASSERT(context != NULL);
  // FIXME(loicm) Add support for unfocused callback.
  // QHybrisScreen* screen = static_cast<QHybrisScreen*>(context);
}

QHybrisIntegration::QHybrisIntegration() {
  // Init ubuntu application UI.
  QStringList args = QCoreApplication::arguments();
  argc_ = args.size() + 1;
  argv_ = new char*[argc_];
  for (int i = 0; i < argc_ - 1; i++)
    argv_[i] = qstrdup(args.at(i).toLocal8Bit());
  argv_[argc_ - 1] = NULL;
  ubuntu_application_ui_init(argc_ - 1, argv_);

  // Create default screen.
  screen_ = new QHybrisScreen();
  screenAdded(screen_);

  // Initialize input.
  if (qEnvironmentVariableIsEmpty("QTHYBRIS_NO_INPUT")) {
    input_ = new QHybrisInput(this);
    inputContext_ = QPlatformInputContextFactory::create();
  } else {
    input_ = NULL;
    inputContext_ = NULL;
  }

  DLOG("QHybrisIntegration::QHybrisIntegration (this=%p)", this);
}

QHybrisIntegration::~QHybrisIntegration() {
  DLOG("QHybrisIntegration::~QHybrisIntegration");
  delete input_;
  delete inputContext_;
  delete screen_;
  for (int i = 0; i < argc_; i++)
    delete [] argv_[i];
  delete [] argv_;
}

QPlatformWindow* QHybrisIntegration::createPlatformWindow(QWindow* window) const {
  DLOG("QHybrisIntegration::createPlatformWindow const (this=%p, window=%p)", this, window);
  return const_cast<QHybrisIntegration*>(this)->createPlatformWindow(window);
}

QPlatformWindow* QHybrisIntegration::createPlatformWindow(QWindow* window) {
  DLOG("QHybrisIntegration::createPlatformWindow (this=%p, window=%p)", this, window);
  static uint sessionType;

  // Start a session before creating the first window.
  static bool once = false;
  if (!once) {
    sessionType = nativeInterface()->property("session").toUInt();
    // FIXME(loicm) Remove that once all system applications have been ported to the new property.
    if (session == 0) {
      sessionType = nativeInterface()->property("ubuntuSessionType").toUInt();
    }
#if !defined(QT_NO_DEBUG)
    ASSERT(sessionType <= SYSTEM_SESSION_TYPE);
    const char* const sessionTypeString[] = {
      "User", "System"
    };
    const char* const stageHintString[] = {
      "Main", "Integration", "Share", "Content picking", "Side", "Configuration",
    };
    const char* const formFactorHintString[] = {
      "Desktop", "Phone", "Tablet"
    };
    LOG("ubuntu session type: '%s'", sessionTypeString[sessionType]);
    LOG("ubuntu application stage hint: '%s'",
        stageHintString[ubuntu_application_ui_setup_get_stage_hint()]);
    LOG("ubuntu application form factor: '%s'",
        formFactorHintString[ubuntu_application_ui_setup_get_form_factor_hint()]);
#endif
    SessionCredentials credentials = {
      static_cast<SessionType>(sessionType), APPLICATION_SUPPORTS_OVERLAYED_MENUBAR, "QtHybris",
      resumedCallback, suspendedCallback, focusedCallback, unfocusedCallback, this
    };
    ubuntu_application_ui_start_a_new_session(&credentials);
    once = true;
  }

  // Create the window.
  QPlatformWindow* platformWindow = new QHybrisWindow(
      window, static_cast<QHybrisScreen*>(screen_), input_, static_cast<bool>(sessionType));
  platformWindow->requestActivateWindow();
  return platformWindow;
}
