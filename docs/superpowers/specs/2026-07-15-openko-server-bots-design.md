# OpenKO 1299 Yerel Sunucu ve Sunucu İçi PK Botları Tasarımı

**Tarih:** 2026-07-15

**Durum:** Kullanıcı tarafından bölüm bölüm onaylandı

**Hedef çalışma dizini:** `C:\Users\hpete\Desktop\KO1453`

## 1. Amaç

Bu çalışma, tek bir gerçek Knight Online istemcisinin tamamen yerel servislerle
(`127.0.0.1`) bağlandığı ve PK alanında istemci açmadan çalışan sunucu içi sanal
oyuncularla savaşabildiği bir geliştirme ortamı kuracaktır.

İlk teslimin başarı tanımı şudur:

- OpenKO 1298/1299 istemcisi yerel hesaba giriş yapabilir.
- Yerel oyun sunucusunda seçilen PK alanına girebilir.
- 5 Karus ve 5 El Morad olmak üzere 10 geçici bot görünür.
- Botlar devriye gezer, en yakın yaşayan düşmanı seçer, yaklaşır ve temel saldırı
  yapar.
- Botlar hasar alabilir, ölebilir ve 15 saniye sonra yeniden doğabilir.
- Botların hiçbirinde ayrı istemci veya ağ bağlantısı bulunmaz.

Bu ilk teslim, daha gelişmiş sınıf yapay zekâsı ve yüzlerce bot için sağlam bir
temel oluşturacaktır.

## 2. Kaynak seçimi ve lisans sınırı

### 2.1 Seçilen temel

Çalışan istemci, sunucu, araçlar, testler ve 1298/1299 veritabanı düzeni aynı
kod tabanında bulunduğu için ana geliştirme tabanı
[Open-KO/KnightOnline](https://github.com/Open-KO/KnightOnline) olacaktır.
Fire-Drake v1453 kaynakları çalışma zamanında bağlanmayacak; yalnızca eski
davranışları ve sunucu akışlarını incelemek için referans olarak saklanacaktır.

OpenKO projesi erken geliştirme aşamasındadır ve kendi README dosyasında gerçek
bir üretim sunucusu için hazır olmadığını açıkça belirtir. Bu çalışma yalnızca
tek bilgisayarda, eğitim ve kişisel geliştirme amacıyla hedeflenmektedir.
README ayrıca projenin başlangıcında sızdırılmış eski kaynaklardan yararlanıldığını
belirtir. Bu köken nedeniyle çalışma ticari dağıtım veya herkese açık sunucu
yayını olarak ele alınmayacaktır.

### 2.2 Dizin düzeni

```text
C:\Users\hpete\Desktop\KO1453\
├── legacy-v1453\       # Fire-Drake v1453 kaynakları; salt referans
└── openko-1299\        # Aktif yerel geliştirme deposu
```

Yerel yardımcı betikler aktif deponun `local/` dizininde tutulacaktır. Böylece
iki bağımsız Git geçmişi korunur ve v1453 kodu yanlışlıkla OpenKO derlemesine
karışmaz.

### 2.3 Dağıtım sınırı

OpenKO kaynak deposu MIT lisanslıdır. Buna karşılık
[Open-KO/ko-client-assets](https://github.com/Open-KO/ko-client-assets) deposu
resmi 1.298 istemci varlıklarını içerir ve görünür bir açık kaynak lisansı
belirtmez. Bu nedenle:

- İstemci varlıkları yalnızca kullanıcının yerel makinesinde kullanılacaktır.
- İstemci varlıkları yeniden paketlenmeyecek, yayınlanmayacak veya başka bir
  depoya eklenmeyecektir.
- Yerel fork veya commit hiçbir şekilde upstream projeye gönderilmeyecektir.
- Tasarım ve bot kodu kaynak varlıklarından ayrı tutulacaktır.

## 3. Kapsam

### 3.1 İlk teslim kapsamı

- Windows üzerinde yerel OpenKO 1298/1299 istemci ve sunucu derlemesi
- SQL Server 2022 Express tabanlı yerel veritabanı
- Tüm servislerin `127.0.0.1` adresine bağlanması
- Ebenezer içinde çalışan soketsiz sanal oyuncular
- 10 botluk temel iki ulus PK döngüsü
- Yönetici komutlarıyla bot ekleme, temizleme, başlatma ve durum sorgulama
- Birim, entegrasyon ve 30 dakikalık yerel dayanıklılık testi

### 3.2 İlk teslim dışında kalanlar

- Party kurma ve party liderini takip etme
- Priest koruma, iyileştirme, buff/debuff ve resurrection
- Warrior, rogue, archer ve mage sınıflarına özel skill zincirleri
- Potion ve item kullanımı
- Town baskını, monument/obelisk savaşı ve koordineli grup taktikleri
- Kalıcı bot karakterleri, envanter veya ekipman kayıtları
- National Point ve kalıcı kill/death istatistikleri
- Zorluk seviyeleri
- 100–500 bot performans hedefi

Bu özellikler temel yaşam döngüsü doğrulandıktan sonra ayrı aşamalar halinde
eklenecektir.

## 4. Sistem mimarisi

```text
WarFare Client
      |
      v
VersionManager / Login
      |
      v
Ebenezer Game Server <----> AIServer
      |                         |
      v                         |
   Aujard <---------------------+
      |
      v
SQL Server 2022 Express

Ebenezer
└── BotManager
    ├── BotSpawner
    ├── BotRegistry
    ├── BotBrain
    ├── TargetSelector
    ├── BotMovement
    └── BotCommandFacade
```

Botların oyuncu görünürlüğü, hedef alınması, hasar alması ve ölümü için gerekli
ana oyun kuralları Ebenezer içindedir. Bu nedenle gelişmiş oyuncu benzeri bot
mantığı ayrı AIServer NPC'leri olarak değil, Ebenezer içindeki soketsiz `CUser`
nesneleri olarak uygulanacaktır.

AIServer mevcut NPC ve dünya davranışlarından sorumlu olmaya devam eder.
BotManager ile v1453 deposu arasında çalışma zamanı bağı bulunmaz.

## 5. Sanal oyuncu modeli

### 5.1 `CBotUser`

`CBotUser`, `CUser` sınıfından türetilen fakat gerçek `TcpServerSocket`
bağlantısı olmayan bir oyuncu nesnesidir.

- Mevcut test amaçlı `CUser(test_tag)` kurulum yolu kullanılacaktır.
- Oyuncu/world alanları mevcut `CUser::Initialize()` akışıyla hazırlanacaktır.
- Soket kimliği yerine ayrılmış sanal oyuncu kimliği atanacaktır.
- `Send()` davranışı botlar için güvenli bir no-op olacaktır; botun istemcisi
  olmadığı için paketin teslim edileceği bir uç yoktur.
- Botun çevresindeki gerçek oyunculara giden paketler mevcut region broadcast
  sistemiyle gönderilmeye devam edecektir.

Bot, mevcut oyuncu veri modelindeki nation, class, level, HP, MP, position,
equipment görünümü ve skill verilerini kullanır. İlk teslimde bu veriler
başlangıç şablonundan bellekte oluşturulur ve SQL'e yazılmaz.

### 5.2 Kimlik alanı

Mevcut gerçek bağlantı kimlikleri `0–2999`, NPC kimlikleri ise `10000` ve
üzerindeki bantta tutulmaktadır. Botlar için aşağıdaki kapalı aralık ayrılır:

```text
Gerçek oyuncular: 0–2999
Sanal botlar:     3000–3499
NPC bandı:        10000+
```

Bu düzen ilk teslimde 10 botu, sonraki aşamalarda ise en fazla 500 botu gerçek
oyuncu ve NPC kimlikleriyle çakışmadan destekler. Kimlik ayırıcı boş kimlikleri
yeniden kullanır; dolu veya sınır dışı kimlik vermez.

Kod düzeyinde gerçek ağ bağlantısı kapasitesi ile toplam oyuncu-nesnesi
kapasitesi ayrılacaktır:

```text
MAX_SOCKET_USER = 3000
MAX_BOT_USER    = 500
MAX_USER        = 3500
```

Ebenezer socket manager yalnızca `MAX_SOCKET_USER` kadar gerçek socket ayırır.
AIServer kullanıcı dizisi ise botlardan gelen mevcut `AG_USER_*` paketlerini
güvenle kabul edebilmek için `MAX_USER` kapasitesini kullanır. Aujard ve
VersionManager botları kalıcı oturum olarak görmeyeceği için 3000 gerçek kullanıcı
sınırında kalır.

### 5.3 `BotRegistry`

`BotRegistry`, sanal kimlikleri `shared_ptr<CBotUser>` nesnelerine eşler.
`EbenezerApp::GetUserPtr()` önce gerçek socket yöneticisini, ardından bot
kaydını sorgular. Böylece region listelerinde yalnızca oyuncu kimliği tutulmaya
devam ederken mevcut `USER_IN`, `USER_OUT`, hareket ve saldırı yayınları botları
da çözebilir.

Mevcut temel saldırı kodundaki doğrudan socket-id doğrulaması, gerçek veya sanal
oyuncuyu `GetUserPtr()` üzerinden çözümleyen tek doğrulamaya dönüştürülecektir.
Bu değişiklik hem gerçek oyuncunun botu hem de soketsiz botun gerçek oyuncuyu
hedeflemesini sağlar; geçersiz kimlik yine null sonuçla reddedilir.

Kayıt erişimi, socket iş parçacıkları ile bot tick akışı arasındaki eşzamanlı
okuma/yazmalara karşı mevcut sunucu kilitleme düzeniyle uyumlu biçimde
korunacaktır. Uzun ömürlü ham hedef işaretçileri tutulmayacaktır.

## 6. BotManager bileşenleri

### 6.1 `BotSpawner`

- Nation, class, level, görünüm ve spawn şablonunu doğrular.
- Sanal kimlik ayırır.
- `CBotUser` nesnesini hazırlar.
- Botu registry, map ve region sistemine kontrollü sırayla ekler.
- Herhangi bir adım başarısız olursa önceki adımların tamamını geri alır.

### 6.2 `BotRegistry`

- Kimlik ayırma ve serbest bırakma
- Bot ekleme, bulma ve kaldırma
- Sunucu kapanışında güvenli toplu temizleme
- Durum ve sayaç üretme

### 6.3 `BotBrain`

Her botun deterministik durum makinesini yürütür. İlk sürümde tüm botlar aynı
temel savaş davranışını kullanır; sınıf alanı gelecekteki strateji seçimi için
korunur.

### 6.4 `TargetSelector`

İlk sürümde adayları şu sırayla filtreler:

1. Aynı zone ve geçerli region kapsamı
2. Yaşayan oyuncu veya bot
3. Karşı nation
4. Hedeflenebilir ve erişilebilir durum
5. En kısa mesafe

Eşit mesafe halinde düşük kullanıcı kimliği seçilerek testlerin deterministik
kalması sağlanır. Gelişmiş hedef puanlama sonraki aşamaya bırakılır.

### 6.5 `BotMovement`

- Devriye noktaları arasında hareket eder.
- Hedef menzil dışındaysa hedefe yaklaşır.
- Hareket verisini mevcut `MoveProcess`/region yayın yoluna verir.
- Geçersiz koordinat veya zone sınırı tespit edilirse güvenli spawn/devriye
  noktasına döner.

### 6.6 `BotCommandFacade`

Yapay zekâ kararlarını mevcut oyuncu komutlarına çevirir. Hareket, temel saldırı,
ölüm ve yeniden doğma için mevcut `CUser` işlem yolları kullanılacaktır. Gerekli
olduğunda gerçek istemcinin üreteceği paket sunucu içinde oluşturulur ve aynı
doğrulama fonksiyonlarına verilir.

Bu katman hasar, menzil, saldırı hızı, HP/MP ve ölüm kurallarını tekrar
uygulamaz; sunucunun yetkili oyun mantığını yeniden kullanır.

## 7. Bot yaşam döngüsü

BotManager, Ebenezer `TimerThread` içinden 200 ms aralıkla tek bot tick akışı
çalıştırır.

```text
Spawn
  |
  v
Patrol ---- düşman yok ----+
  |                        |
  v                        |
SelectTarget               |
  |                        |
  v                        |
Approach ------------------+
  |
  v
BasicAttack
  |  hedef geçersiz/öldü
  +-----------------------> Patrol
  |
  | bot öldü
  v
Dead -- 15 saniye --> Respawn --> Patrol
```

Her tick başlangıcında botun kendisi, zone'u ve hedef kimliği yeniden çözülür.
Hedef ölü, kayıp, aynı nation'a dönüşmüş veya zone dışına çıkmışsa hedef temizlenir.
Saldırı yalnızca mevcut sunucu cooldown ve menzil kontrolleri izin verdiğinde
gerçekleşir.

Rastgele devriye seçimi testlerde sabit seed ile, gerçek çalışmada yerel seed ile
yürütülür.

## 8. Yönetici komutları ve yapılandırma

İlk teslim aşağıdaki komutları sunacaktır:

```text
+bot_add karus warrior 5
+bot_add elmorad warrior 5
+bot_remove_all
+bot_start_pk
+bot_status
```

Komutlar yalnızca yetkili yönetici kullanıcı tarafından çalıştırılır. Geçersiz
nation, class, sayı veya dolu kimlik bandı açık hata üretir ve kısmi bot grubu
oluşturmaz.

Bot ayarları Ebenezer yapılandırmasına veya ayrılmış bir `BotServer.ini`
dosyasına aşağıdaki anlamlarla eklenir:

```ini
[BOTS]
Enabled=1
Count=10
TickMilliseconds=200
RespawnSeconds=15
Zone=<dogrulanmis-pk-zone-id>
```

Zone kimliği ve spawn koordinatları OpenKO harita/veritabanı içeriği incelenerek
doğrulanacaktır. Ronark Land verisi çalışır durumdaysa varsayılan PK alanı olarak
seçilir; aksi halde mevcut çalışan PK alanı kullanılır ve bu durum belgelenir.

## 9. Yerel Windows kurulumu

### 9.1 Araç zinciri

- Visual Studio 2022 Build Tools veya Visual Studio 2022
- MSVC v143 C++ araçları
- Windows SDK
- ATL/MFC bileşenleri
- İlgili CMake araçları
- SQL Server 2022 Express
- Microsoft SQL ODBC sürücüsü

Docker zorunlu değildir. Windows istemcisi ve mevcut ODBC sunucu akışı için
yerel SQL Server Express tercih edilmiştir.

### 9.2 Yerel yardımcı betikler

Aktif depoda aşağıdaki betikler kullanılır:

```text
local/Setup-Database.ps1
local/Build-Local.ps1
local/Start-Local.ps1
local/Stop-Local.ps1
```

- `Setup-Database.ps1`: önkoşulları ve SQL bağlantısını doğrular, yerel ayar
  şablonlarını hazırlar.
- `Build-Local.ps1`: desteklenen Windows yapılandırmasını derler ve testleri
  çalıştırır.
- `Start-Local.ps1`: bağımlılık sırasına göre servisleri ve istemciyi gizli
  süreçler olarak başlatır; stdout/stderr loglarını ve yalnızca oluşturduğu
  süreçlerin kimliklerini atomik `local/pids.json` durumunda tutar.
- `Stop-Local.ps1`: durum şemasını, executable yolu/adı ve süreç başlangıç
  zamanını doğrular; yalnızca kaydedilmiş kesin PID'leri ters sırada kapatır.
  Süreç adına göre tarama veya toplu kapatma yapmaz.

Parolalar commit edilmez. Örnek yapılandırmalar yalnızca yer tutucu içerir;
gerçek yerel değerler ignore edilen dosyalarda tutulur.

### 9.3 Başlatma sırası

```text
1. SQL Server 2022 Express
2. Aujard / VersionManager / ItemManager / AIServer
3. Ebenezer
4. WarFare istemcisi
```

Her servis yalnızca zorunlu bağımlılığı hazır olduğunda başlatılır. İstemci
sunucu adresi ve tüm dahili servis adresleri `127.0.0.1` olarak ayarlanır.

## 10. Hata yönetimi

### 10.1 Başlangıç hataları

- SQL, harita veya zorunlu tablo eksikse bağımlı servis başlatılmaz ve açık log
  üretilir.
- Bot yapılandırması geçersizse Ebenezer çalışmaya devam eder; yalnızca bot
  alt sistemi kapatılır.
- Zone veya spawn koordinatları doğrulanamazsa bot üretimi başlamaz.

### 10.2 Bot oluşturma ve çalışma hataları

- Bot oluşturma işlemi kimlik, nesne, registry, map ve region adımları açısından
  geri alınabilir bir işlem olarak ele alınır.
- Hedefler her tick kimlikten yeniden çözülür; kalıcı ham hedef işaretçisi
  tutulmaz.
- Hedef kaybolursa bot `Patrol` durumuna döner.
- Tek botta beklenmeyen hata oluşursa bot kimliği ve durumu loglanır; bot
  region'dan çıkarılıp güvenli biçimde kaldırılır. Manager diğer botları
  çalıştırmaya devam eder.
- Aynı hatayı üreten bot şablonu tekrar tekrar otomatik oluşturulmaz.

### 10.3 Kapanış

1. Yeni bot tick'leri durdurulur.
2. Botlar için `USER_OUT`/region temizliği tamamlanır.
3. BotRegistry boşaltılır ve kimlikler serbest bırakılır.
4. Ardından map, socket manager ve diğer sunucu bileşenleri kapanır.

İlk sürümde botlar SQL'e yazılmadığı için yarım kalmış bot karakteri veya
envanter kaydı oluşmaz.

## 11. Test stratejisi

### 11.1 Birim testleri

- Kimlik ayırıcı yalnızca `3000–3499` aralığını verir.
- Dolu kimlik atlanır; kaldırılan kimlik tekrar kullanılabilir.
- TargetSelector aynı nation, ölü, hedeflenemez veya başka zone'daki adayları
  eler.
- En yakın geçerli düşman deterministik seçilir.
- Durum makinesi devriye, yaklaşma, saldırı, ölüm ve 15 saniyelik respawn
  geçişlerini sahte saatle doğrular.
- `CBotUser::Send()` soketsiz durumda güvenli no-op olur.

### 11.2 Entegrasyon testleri

- Socket yöneticisi gerçek oyuncuyu, BotRegistry sanal oyuncuyu çözer.
- Soketsiz bot map ve region'a eklenip güvenli biçimde çıkarılabilir.
- Bot hareketi gerçek oyuncular için mevcut region paketini üretir.
- Gerçek oyuncu botu hedefleyip hasar verebilir.
- İki nation botu birbirine hasar verir, ölür ve yeniden doğar.
- Bot kaldırıldıktan sonra region içinde eski kimlik kalmaz.

### 11.3 Yerel uçtan uca test

1. SQL ve tüm yerel servisler başlatılır.
2. WarFare istemcisiyle yerel hesapta oturum açılır.
3. Gerçek oyuncu yapılandırılmış PK alanına girer.
4. 5 Karus ve 5 El Morad botun görünürlüğü doğrulanır.
5. Botların hareket, hedef seçimi, saldırı, ölüm ve yeniden doğma döngüleri
   gözlenir.
6. Gerçek oyuncunun botu hedefleyip öldürebildiği doğrulanır.
7. Sistem 30 dakika çalıştırılır.

30 dakikalık test sonunda şu koşullar sağlanmalıdır:

- Ebenezer veya bağlı servislerde çökme yok.
- Sanal kimlik sızıntısı veya çakışma yok.
- Region listelerinde silinmiş bot kimliği yok.
- Botlar kalıcı olarak bozuk durumda takılı kalmıyor.
- `+bot_status` kayıt sayısı ile dünyadaki canlı/ölü bot toplamı tutarlı.

## 12. İlk teslim kabul ölçütleri

İlk teslim ancak aşağıdakilerin tamamı sağlandığında tamamlanmış kabul edilir:

- Yerel istemci `127.0.0.1` servislerine bağlanıp oyuna girebilir.
- PK alanında tam 10 soketsiz bot görünür.
- Her iki nation temsil edilir: 5 Karus, 5 El Morad.
- Botlar hareket eder, en yakın geçerli düşmanı seçer ve temel saldırı yapar.
- Botlar gerçek oyuncuyla aynı yetkili hasar/ölüm kurallarından geçer.
- Ölümden 15 saniye sonra güvenli respawn gerçekleşir.
- Dört yönetici komutu çalışır: add, remove-all, start-pk ve status.
- Otomatik testler geçer.
- 30 dakikalık yerel dayanıklılık testi kabul koşullarını karşılar.

## 13. Sonraki aşamalar

Temel teslim doğrulandıktan sonra geliştirme aşağıdaki sırayla genişletilebilir:

1. Potion, skill cooldown ve sınıfa özel tekil davranışlar
2. Party oluşturma, lider takibi ve priest koruma
3. Gelişmiş hedef puanlama ve geri çekilme/yeniden gruplanma
4. Ekipman şablonları, isimler, clan görünümü ve istatistikler
5. Monument/town hedefleri ve koordineli PK
6. Zorluk seviyeleri
7. Profiling, spatial sorgu iyileştirmeleri ve kademeli 100–500 bot ölçekleme

Her aşama bir önceki aşamanın testlerini koruyacak ve ayrı kabul ölçütleriyle
planlanacaktır.

## 14. Doğrulama durumu

Başlatma/durdurma betiklerinin PowerShell AST/statik kontrolleri ve gerçek oyun
servislerini kullanmayan geçici bir sahte süreç sahipliği testi uygulanmıştır.
Bu kanıt yalnızca operasyon betiklerinin dar güvenlik davranışını kapsar.

Debug/Release tam derleme ve test çıktıları, gerçek yerel servis başlatma,
istemciyle oturum açma, on botun oyun içi davranış gözlemleri ve 30 dakikalık
dayanıklılık kaydı denetleyici tarafından henüz çalıştırılmamıştır. Bu kanıtlar
elde edilene kadar Bölüm 12'deki ilk teslim kabulü tamamlanmış sayılmaz.
