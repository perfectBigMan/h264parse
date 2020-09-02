#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFileDialog>
#include <vector>
#include <QTableWidgetItem>
#include "MediaFile.h"
#include "decoder.h"
#include "YuvToRGB.h"

namespace Ui {
class Widget;
}




class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();


public slots:
    void openFile();
    void onItemClicked(QTableWidgetItem* item);

private:
    void init();
    void initHexView();
    void showHexView(Frame* frame);
    void showFileinfo();
    void addRow(int seq, int offset, int len,
                QString startCode, QString typedesc, QString info,
                Qt::GlobalColor color);

private:
    Ui::Widget *ui;
    QString mFile;
    MediaFile* mMediaFile;
    YuvToRGB* yuvToRGB = NULL;
};

#endif // WIDGET_H
