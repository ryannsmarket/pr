#ifndef IMPORTMIDI_DELEGATE_H
#define IMPORTMIDI_DELEGATE_H


namespace Ms {

class OperationsDelegate : public QStyledItemDelegate
      {
      Q_OBJECT

   public:
      explicit OperationsDelegate(QWidget *appWindow, bool rightArrowAlign);
      void paint(QPainter *painter,
                 const QStyleOptionViewItem &option,
                 const QModelIndex &index) const;
      QWidget* createEditor(QWidget *parent,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const;
      void setEditorData(QWidget *editor, const QModelIndex &index) const;
      void setModelData(QWidget *editor, QAbstractItemModel *model,
                        const QModelIndex &index) const;

   private slots:
      void commitAndCloseEditor();

   private:
      QWidget *appWindow;
      const bool rightArrowAlign;
      };

} // namespace Ms


#endif // IMPORTMIDI_DELEGATE_H
