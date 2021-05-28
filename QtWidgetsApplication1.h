#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/qabstractitemview.h>

#include "ui_QtWidgetsApplication1.h"



class QtWidgetsApplication1: public QMainWindow
{
Q_OBJECT

public:
    QtWidgetsApplication1(QWidget* parent = Q_NULLPTR);

private:

    Ui::QtWidgetsApplication1Class ui;

    //std::unique_ptr<QAbstractTableModel> mz_table;
 //std::unique_ptr<QAbstractItemView>    view;
};
