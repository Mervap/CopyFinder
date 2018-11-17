#ifndef MYTREEWIDGET_H
#define MYTREEWIDGET_H

#include <QTreeWidget>

class MyQTreeWidget : public QTreeWidget {
public:
    MyQTreeWidget(QWidget *parent) : QTreeWidget(parent) {}

    signals:
            void
    itemClicked(QTreeWidgetItem
    *item);

public:
    void contextMenuEvent(QContextMenuEvent *event) {
        emit itemClicked(currentItem());
    }
};

#endif // MYTREEWIDGET_H
