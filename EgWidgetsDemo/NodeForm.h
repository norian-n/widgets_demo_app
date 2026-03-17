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

class EgNodeWidget;

class NodeForm :  public QDialog
{
    Q_OBJECT

public:
    NodeForm(QWidget *parent = 0);

    EgNodeWidget* dataNodeWidget    {nullptr};

    void openNode();

private slots:
    void on_cancelButton_clicked();
    void on_okButton_clicked();

private:
    Ui::NodeForm *ui;
};

