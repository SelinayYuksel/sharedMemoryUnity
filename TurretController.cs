using UnityEngine;
using System.IO.MemoryMappedFiles;
using System;

public class TurretController : MonoBehaviour
{
    [Header("1. Model Parçaları")]
    public Transform platform; // Base objesi sürüklenecek
    public Transform namlu;    // Gun objesi sürüklenecek

    [Header("2. Ayarlar")]
    public float donusHizi = 5f; // Yumuşak geçiş hızı

    [Header("3. Test Modu (Qt Yokken Test İçin)")]
    public bool unityIcindenTestEt = true; // Bu tikliyse bellek yerine alttaki sliderları okur
    [Range(-180f, 180f)] public float testPanAcisi = 0f; // Sağa sola dönüş testi
    [Range(-45f, 45f)] public float testTiltAcisi = 0f;  // Aşağı yukarı dönüş testi

    // Paylaşımlı Bellek Değişkenleri
    private MemoryMappedFile mmf;
    private MemoryMappedViewAccessor accessor;
    private readonly string sharedMemoryName = "QtUnitySharedMem";
    private readonly int memorySize = 8; // Toplam 8 byte (2 float)

    private float hedefPanAcisi = 0f;
    private float hedefTiltAcisi = 0f;

    void Start()
    {
        // Uygulama başladığında belleği oluşturmayı veya bağlanmayı dene
        try
        {
            mmf = MemoryMappedFile.CreateOrOpen(sharedMemoryName, memorySize);
            accessor = mmf.CreateViewAccessor(0, memorySize);
            Debug.Log("Paylaşımlı Bellek Sistemi Hazır!");
        }
        catch (Exception e)
        {
            Debug.LogError("Bellek açılamadı: " + e.Message);
        }
    }

    void Update()
    {
        // Hangi veriyi okuyacağımıza karar veriyoruz (Test mi, Qt belleği mi?)
        if (unityIcindenTestEt)
        {
            // Unity arayüzündeki test slider'larından oku
            hedefPanAcisi = testPanAcisi;
            hedefTiltAcisi = testTiltAcisi;
        }
        else if (accessor != null)
        {
            // Qt'nin yazdığı işletim sistemi belleğinden oku
            hedefPanAcisi = accessor.ReadSingle(0);
            hedefTiltAcisi = accessor.ReadSingle(4);
        }

        // Modeli İstenen Açılara Yumuşakça Döndür
        // Platform Y ekseninde (Pan) döner
        Quaternion panRotation = Quaternion.Euler(0, hedefPanAcisi, 0);
        platform.localRotation = Quaternion.Lerp(platform.localRotation, panRotation, Time.deltaTime * donusHizi);

        // Namlu X ekseninde (Tilt) döner (Modeline göre bu Z ekseni de olabilir, testte anlarız)
        Quaternion tiltRotation = Quaternion.Euler(hedefTiltAcisi, 0, 0);
        namlu.localRotation = Quaternion.Lerp(namlu.localRotation, tiltRotation, Time.deltaTime * donusHizi);
    }

    void OnApplicationQuit()
    {
        // Program kapanırken belleği temizle ki RAM'de asılı kalmasın
        if (accessor != null) accessor.Dispose();
        if (mmf != null) mmf.Dispose();
    }
}