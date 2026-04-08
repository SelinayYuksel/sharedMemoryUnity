#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QWheelEvent>
#include <QMainWindow>
#include <QSharedMemory>
#include <QPoint>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow(); // Yıkıcı fonksiyonumuz burada tanımlı
protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
private:
    Ui::MainWindow *ui;

    QSharedMemory sharedMem;

    float guncelPan = 0.0f;
    float guncelTilt = 0.0f;
    float guncelZoom = 60.0f;
    QPoint sonFareKonumu;

    void bellegeYaz(); // Belleğe yazma fonksiyonumuz burada tanımlı
};
#endif // MAINWINDOW_H
