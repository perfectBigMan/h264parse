#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <QDebug>



Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    mMediaFile(NULL)
{
    ui->setupUi(this);
    init();
}

Widget::~Widget()
{
    if (mMediaFile) {
        delete mMediaFile;
        mMediaFile = NULL;
    }

    delete ui;
}


void Widget::openFile() {
    mFile = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择文件"));
    if (mFile.isEmpty())return;

    this->setWindowTitle(mFile);
    if (mMediaFile) {
        delete mMediaFile;
    }
    if (yuvToRGB) {
        delete yuvToRGB;
    }


    ui->tableWidget->setRowCount(0);
    mMediaFile = new MediaFile(mFile);
    mMediaFile->parse();
    std::vector<Frame*>* list = mMediaFile->getFrameList();
    int picFrameIndex = 0;
    for(unsigned int i = 0; i < list->size(); i++) {
        Frame* f = (*list)[i];
        int naltype = f->getNalType();
        QString info = f->getInfo();
        if (naltype == 1 || naltype == 5) {
            info += " #" + QString::number(picFrameIndex);
            picFrameIndex++;
        }
        addRow(f->getSeq(), f->getOffset(), f->getLen(), f->getStartCode(), f->getTypedesc(), info, f->getColor());
    }


    showFileinfo();
    yuvToRGB = new YuvToRGB();
    if (yuvToRGB) {
        yuvToRGB->init(Frame::width, Frame::height);
    }
}

void Widget::onItemClicked(QTableWidgetItem* item) {
    qDebug() << "onItemClicket" << item->row();
    Frame* frame = mMediaFile->getFrame(item->row());
    frame->parse();
    showHexView(frame);

    if (mMediaFile->parseFrameToYuv(frame) == 0) {
        yuvToRGB->convert(frame->getYuvData(), frame->getYuvLen());
        QImage image(yuvToRGB->getRgbBuffer(), Frame::width, Frame::height, QImage::Format_RGBA8888);
        QPixmap pixmapimg = QPixmap::fromImage(image);
        pixmapimg = pixmapimg.scaled(ui->frameImage->size(),Qt::KeepAspectRatio);
        ui->frameImage->setPixmap(pixmapimg);

        frame->releaseYuvData();
    } else {
        ui->frameImage->clear();
    }

}

 void Widget::init() {
    connect(ui->openfile, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(ui->tableWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onItemClicked(QTableWidgetItem*)));

    int width = this->geometry().width();
    int height = this->geometry().height();
    this->setFixedSize(width,height); //设置窗体固定大小


    ui->tableWidget->verticalHeader()->setDefaultSectionSize(15);

    ui->tableWidget->setColumnWidth(0, 50); //no
    ui->tableWidget->setColumnWidth(1, 80); //offset
    ui->tableWidget->setColumnWidth(2, 50); //len
    ui->tableWidget->setColumnWidth(3, 80); //start
    ui->tableWidget->setColumnWidth(4, 250); //type
    ui->tableWidget->setColumnWidth(5, 100); //info

    ui->tableWidget->setStyleSheet("selection-background-color: blue");

    initHexView();

 }

 void Widget::initHexView() {
    ui->hexview->setColumnCount(17);
//    ui->hexview->setRowCount(2);
    ui->hexview->verticalHeader()->setDefaultSectionSize(15);
    ui->hexview->horizontalHeader()->setDefaultSectionSize(20);
    ui->hexview->setColumnWidth(0, 70); //no
    QStringList sListHeader;
    sListHeader << " " << "0" << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "A" << "B" << "C" << "D" << "E" << "F";
    ui->hexview->setHorizontalHeaderLabels(sListHeader);
    ui->hexview->verticalHeader()->setVisible(false);
    ui->hexview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->hexview->setShowGrid(false);
 }
 void Widget::showHexView(Frame* frame) {
     ui->hexview->setRowCount(0);

     int offset = frame->getOffset();
     int len = frame->getLen();
     int rownum = len / 16;
     if (len % 16) {
         rownum++;
     }
//     qDebug() << "rownum:" << rownum;

     for(int j = 0; j < rownum; j++) {
         int row = ui->hexview->rowCount();
         ui->hexview->insertRow(row);

         QTableWidgetItem* item = new QTableWidgetItem(QString("%1").arg(j * 16, 8, 16, QLatin1Char('0')).toUpper());
         ui->hexview->setItem(j, 0, item);

         for(int i = 0; i < 16; i++) {
             if (j * 16 + i == len) {
                 break;
             }
             unsigned char byte = mMediaFile->getByte(offset + j * 16 + i);
             QTableWidgetItem* item = new QTableWidgetItem(QString("%1").arg(byte, 2, 16, QLatin1Char('0')).toUpper());
 //            QTableWidgetItem* item = new QTableWidgetItem(QString(byte));
             item->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
             ui->hexview->setItem(j, 1 + i, item);
         }
     }
 }

 void Widget::showFileinfo() {
     QLabel* resolution = (QLabel*)ui->fileinfo->itemAt(0, QFormLayout::FieldRole)->widget();
     QString val = QString("%1 x %2").arg(Frame::width).arg(Frame::height);
     resolution->setText(val);

     QLabel* cl = (QLabel*)ui->fileinfo->itemAt(1, QFormLayout::FieldRole)->widget();
     QLabel* cr = (QLabel*)ui->fileinfo->itemAt(2, QFormLayout::FieldRole)->widget();
     QLabel* ct = (QLabel*)ui->fileinfo->itemAt(3, QFormLayout::FieldRole)->widget();
     QLabel* cb = (QLabel*)ui->fileinfo->itemAt(4, QFormLayout::FieldRole)->widget();
     cl->setText(QString::number(Frame::frameCropLeftOffset));
     cr->setText(QString::number(Frame::frameCropRightOffset));
     ct->setText(QString::number(Frame::frameCropTopOffset));
     cb->setText(QString::number(Frame::frameCropBottomOffset));

     QLabel* videoformat = (QLabel*)ui->fileinfo->itemAt(5, QFormLayout::FieldRole)->widget();
     QLabel* streamtype = (QLabel*)ui->fileinfo->itemAt(6, QFormLayout::FieldRole)->widget();
     streamtype->setText(QString("Baseline Profile @ Level %1").arg(Frame::levelIdc));
     QLabel* encodingtype = (QLabel*)ui->fileinfo->itemAt(7, QFormLayout::FieldRole)->widget();
     QLabel* fps = (QLabel*)ui->fileinfo->itemAt(8, QFormLayout::FieldRole)->widget();

 }

 void Widget::addRow(int seq, int offset, int len,
                    QString startCode, QString typedesc, QString info,
                    Qt::GlobalColor color) {
     //qDebug() << seq << " " << offset << " " << len;



     int row = ui->tableWidget->rowCount();
     ui->tableWidget->insertRow(row);

     QTableWidgetItem* item;
     item = new QTableWidgetItem(QString::number(seq));
     item->setBackgroundColor(QColor(color));
     ui->tableWidget->setItem(row, 0, item);

//     item = new QTableWidgetItem(QString::number(offset, 16));
     item = new QTableWidgetItem(QString("%1").arg(offset, 8, 16, QLatin1Char('0')).toUpper());
     item->setBackgroundColor(QColor(color));
     ui->tableWidget->setItem(row, 1, item);


     item = new QTableWidgetItem(QString::number(len));
     item->setBackgroundColor(QColor(color));
     ui->tableWidget->setItem(row, 2, item);

     item = new QTableWidgetItem(startCode);
     item->setBackgroundColor(QColor(color));
     ui->tableWidget->setItem(row, 3, item);

     item = new QTableWidgetItem(typedesc);
     item->setBackgroundColor(QColor(color));
     ui->tableWidget->setItem(row, 4, item);

     item = new QTableWidgetItem(info);
     item->setBackgroundColor(QColor(color));
     ui->tableWidget->setItem(row, 5, item);


 }
