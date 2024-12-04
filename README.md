# Weather Information Socket Application 🌦️

Bu proje, bir sunucu ve istemci uygulamasını kullanarak şehir bazında hava durumu verilerini almak için tasarlanmış bir socket programlama örneğidir. Proje, **C** programlama dili ile yazılmış ve **OpenWeather API** kullanılarak hava durumu verilerine erişim sağlanmıştır.

---

## Proje Özellikleri
- **Sunucu Uygulaması:** İstemciden gelen şehir adını alır, OpenWeather API üzerinden hava durumu verilerini çeker ve istemciye gönderir.
- **İstemci Uygulaması:** Kullanıcıdan şehir adı alır, bu bilgiyi sunucuya gönderir ve dönen hava durumu verilerini ekrana yazdırır.
- **Çoklu İstemci Desteği:** Sunucu, birden fazla istemciyi aynı anda yönetebilir.

---

## Kullanılan Teknolojiler
- **C Socket Programming:** İstemci ve sunucu arasındaki iletişim için.
- **pthread:** Çoklu istemci desteği için.
- **libcurl:** OpenWeather API'sinden veri çekmek için.
- **cJSON:** JSON verilerini ayrıştırmak için.

---

## Gereksinimler
Projeyi çalıştırmadan önce aşağıdaki bağımlılıkların sisteminizde kurulu olduğundan emin olun:
- **libcurl**
- **cJSON**

### MacOS/Linux Bağımlılık Yükleme:
```bash
sudo apt-get install libcurl4-openssl-dev  # Ubuntu için
brew install cjson curl                   # MacOS için

# Projenin Çalıştırılması

## 1. Sunucu Uygulamasını Derleme ve Çalıştırma
```bash
gcc -o server server.c -pthread -lcurl -lcjson
./server

## 2.İstemci Uygulamasını Derleme ve Çalıştırma
gcc -o client client.c
./client

## 3.Kullanım
İstemci çalıştırıldığında bir şehir adı girmeniz istenir.
Sunucu, şehir adını alır ve OpenWeather API'sinden hava durumu bilgilerini çeker.
İstemci, dönen hava durumu bilgisini ekrana yazdırır.


### Projenin yapısı
.
├── server.c        # Sunucu uygulaması
├── client.c        # İstemci uygulaması
└── README.md       # Proje açıklama dosyası
