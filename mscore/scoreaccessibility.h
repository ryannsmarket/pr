#ifndef __SCORE_ACCESSIBILITY__
#define __SCORE_ACCESSIBILITY__

#include <QStatusBar>
#include <QAccessible>
#include <QAccessibleWidget>
#include "scoreview.h"

namespace  Ms {

//---------------------------------------------------------
//   AccessibleScoreView
//---------------------------------------------------------

#define SCOREVIEW_VALUEINTERFACE

#ifdef SCOREVIEW_VALUEINTERFACE
class AccessibleScoreView : public QObject, QAccessibleWidget, QAccessibleValueInterface {
#else
class AccessibleScoreView : public QObject, QAccessibleWidget {
#endif
      Q_OBJECT
      ScoreView* s;

   public:
      AccessibleScoreView(ScoreView* c);
      int childCount() const Q_DECL_OVERRIDE;
      QAccessibleInterface* child(int /*index*/) const Q_DECL_OVERRIDE;
      QAccessibleInterface* parent() const Q_DECL_OVERRIDE;
      QRect rect() const Q_DECL_OVERRIDE;
      bool isValid() const Q_DECL_OVERRIDE;
      //QAccessible::State state() const Q_DECL_OVERRIDE;
      QAccessible::Role role() const Q_DECL_OVERRIDE;
      QString text(QAccessible::Text t) const Q_DECL_OVERRIDE;
      QWindow* window() const  Q_DECL_OVERRIDE;
      static QAccessibleInterface* ScoreViewFactory(const QString &classname, QObject *object);
#ifdef SCOREVIEW_VALUEINTERFACE
      virtual void* interface_cast(QAccessible::InterfaceType t) Q_DECL_OVERRIDE;
      virtual void setCurrentValue(const QVariant&) Q_DECL_OVERRIDE;
      virtual QVariant currentValue() const Q_DECL_OVERRIDE;
      virtual QVariant maximumValue() const Q_DECL_OVERRIDE;
      virtual QVariant minimumValue() const Q_DECL_OVERRIDE;
      virtual QVariant minimumStepSize() const Q_DECL_OVERRIDE;
#endif
      };

//---------------------------------------------------------
//   ScoreAccessibility
//---------------------------------------------------------

class ScoreAccessibility : public QObject {
      Q_OBJECT

      static ScoreAccessibility* inst;
      QMainWindow* mainWindow;
      QLabel* statusBarLabel;
      ScoreAccessibility(QMainWindow* statusBar);
      std::pair<int, float>barbeat(Element* e);
      int _oldStaff = -1;
      int _oldBar = -1;

   public:
      ~ScoreAccessibility();
      void updateAccessibilityInfo();
      void clearAccessibilityInfo();
      static void createInstance(QMainWindow* statusBar);
      static ScoreAccessibility* instance();
      void currentInfoChanged();
      static void makeReadable(QString&);
      };

}

#endif
