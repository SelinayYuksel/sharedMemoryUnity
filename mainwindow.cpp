#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QProcess>
#include <QWindow>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setMouseTracking(true); // Fare tıklanmasa bile hareketleri izle
    ui->centralwidget->setMouseTracking(true);
    ui->mouseLayer->raise();
    ui->mouseLayer->setCursor(Qt::OpenHandCursor);

    // 1. Paylaşımlı Belleği Hazırla
    sharedMem.setNativeKey("QtUnitySharedMem");
    if (!sharedMem.attach()) {
        sharedMem.create(12); // Eğer bağlı değilse 12 byte olarak oluştur
    }

    // Eğer bellek oluşturulamazsa (çünkü zaten varsa), olan belleğe bağlan!
    if (!sharedMem.create(12)) {
        if (!sharedMem.attach()) {
            qDebug() << "KRITIK HATA: Bellege baglanilamadi!";
        } else {
            qDebug() << "Eski bellege basariyla baglanildi.";
        }
    } else {
        qDebug() << "Yeni bellek basariyla olusturuldu.";
    }

    // 2. Unity'yi Qt İçine Gömme İşlemi
    WId containerId = ui->unityContainer->winId();
    QString hwndString = QString::number((qintptr)containerId);

    // KENDİ UNITY .EXE YOLUNU BURAYA YAZ
    QString unityPath = "C:/Users/ORTAK/Downloads/cikti/myProject.exe";

    QStringList args;
    args << "-parentHWND" << hwndString;
    args << "-screen-width" << QString::number(ui->unityContainer->width());
    args << "-screen-height" << QString::number(ui->unityContainer->height());

    QProcess *unityProc = new QProcess(this);
    unityProc->start(unityPath, args);
    ui->unityContainer->setAttribute(Qt::WA_TransparentForMouseEvents);
    // Eğer Unity başlatılamazsa alttaki Output (Application Output) paneline nedenini yazar
    connect(unityProc, &QProcess::errorOccurred, [=](QProcess::ProcessError error){
        qDebug() << "Unity Baslatilamadi! Hata Kodu:" << error << "-" << unityProc->errorString();
    });

    // 3. Slider'ları Belleğe Bağla
    connect(ui->horizontalSlider_Pan, &QSlider::valueChanged, this, [=](int val){
        this->guncelPan = (float)val;
        this->bellegeYaz();
    });

    connect(ui->verticalSlider_Tilt, &QSlider::valueChanged, this, [=](int val){
        this->guncelTilt = (float)val;
        this->bellegeYaz();
    });
}

// YIKICI FONKSİYON GÖVDESİ (Eksik olan buydu)
MainWindow::~MainWindow()
{
    sharedMem.detach();
    delete ui;
}

// BELLEĞE YAZMA FONKSİYONU GÖVDESİ (Eksik olan buydu)
void MainWindow::bellegeYaz()
{
    if (!sharedMem.isAttached()) return;

    sharedMem.lock();

    float *hedefBellek = static_cast<float*>(sharedMem.data());
    hedefBellek[0] = guncelPan;
    hedefBellek[1] = guncelTilt;

    // YENİ: 3. değeri belleğe yaz
    hedefBellek[2] = guncelZoom;

    sharedMem.unlock();
}
void MainWindow::wheelEvent(QWheelEvent *event)
{
    // Tekerlek yönüne göre zoom değerini değiştir
    if (event->angleDelta().y() > 0)
        guncelZoom -= 3.0f; // İleri -> Yakınlaş
    else
        guncelZoom += 3.0f; // Geri -> Uzaklaş

    // Sınırları koru (Çok yaklaşınca görüntü bozulmasın)
    guncelZoom = qBound(10.0f, guncelZoom, 90.0f);

    bellegeYaz();
}
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    // Tıklanan yer mouseLayer üzerindeyse veya genel penceredeyse konumu al
    sonFareKonumu = event->pos();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    // Sol tık basılıyken döndürme
    if (event->buttons() & Qt::LeftButton) {
        QPoint fark = event->pos() - sonFareKonumu;

        guncelPan += fark.x() * 0.2f;
        guncelTilt -= fark.y() * 0.2f;

        // Sliderları da güncelle ki hareket ettiğini görelim
        ui->horizontalSlider_Pan->setValue((int)guncelPan);
        ui->verticalSlider_Tilt->setValue((int)guncelTilt);

        bellegeYaz();
    }
    // Sağ tık basılıyken Zoom
    else if (event->buttons() & Qt::RightButton) {
        QPoint fark = event->pos() - sonFareKonumu;
        guncelZoom += fark.y() * 0.5f;
        guncelZoom = qBound(10.0f, guncelZoom, 90.0f);

        bellegeYaz();
    }

    sonFareKonumu = event->pos();
}
