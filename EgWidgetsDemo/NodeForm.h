/*
 * EcoGraphDB - Exo Cortex Graph Database Engine
 *
 * Copyright (c) 2020 Dmitry 'Norian' Solodkiy
 *
 * License: defined in license.txt file located in the root sources dir
 *
 */

#pragma once

#include <QDialog>

#include "ui_NodeForm.h"

class EgGraphWidget;
class EgNodeWidget;

class NodeForm :  public QDialog
{
    Q_OBJECT

public:
    NodeForm(QWidget *parent = 0);

    EgGraphWidget* GraphWidget;
    EgNodeWidget* dataNodeWidget    {nullptr};
    QString imagePath;

    void openNode();
    // void loadImage();

private slots:
    void on_cancelButton_clicked();
    void on_okButton_clicked();

    void on_imageButton_clicked();

    void on_fillButton_clicked();

private:
    Ui::NodeForm *ui;
};

