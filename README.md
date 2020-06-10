# Jtalk_Server

Aşağıdaki linkten ulaşılabilecek ödevin çözümüdür : 

http://web.eecs.utk.edu/~huangj/cs360/360/labs/lab8/lab.html

----------------------------------------------------------------------------------------------------------

Jtalk_server, jtalk client'lari icin ayni anda birden fazla baglantiya imkan veren bir sunucudur.
Programın genel calisma akisi su sekildedir:
Oncelikle main fonksiyonunda baglantilari tutmak icin gerekli olan JRB agaci olusturulur.
Arguman olarak alinan hostname ve socket numarasi ile bir socket acilir. İleride baglantilar
sonlandirildiginda ise yarayacak olan sigpipe handler belirtilir. Sunucunu surekli yeni baglantilara
cevap vermesini saglayacak olan seek_connections fonskiyonu bir thread ile fork edilir. Bu fonksiyon
sunucu acik oldugu surece calisacaktir. En son olarak da sunucuyu kontrol icin bir arayuz olan jtalk
console bir thread ile fork edilir ve jtalk console'un calismayi durdurmasi beklenir.
Seek connections methodu, yukarida belirtilen islevi gerceklestirir ve her baglanti geldiginde
kullanicinin login,logout olması ve mesaj gondermesini saglayacak olan simpcat fonksiyonuna o
baglanti icin bir thread fork eder.
Simpcat(ismini degistirmedik)'in temel gorevi baglantinin fd'sinden gelen her bir karakteri
baglanti agacindaki tum baglanti fd'lerine gondermektir. Bir baglanti acildiginda gonderilen ilk veriler
"kullanici ismi:" seklinde olacagindan bu baglantiyi tutmak icin : karakteri okunana kadar gelen
karakterleri isim olarak kaydeder ve yeni bir baglanti kontrol struct'i olusturup fd ile birlikte baglanti
agacina ekler. Baglanti agaciyla ilgili tum islemlerde global olarak tutulan mutek kilidi kitlenir ve islem
bitince kilit acilir.
Eger bir kullanici cikis yapmis ise bir sonraki yazma isleminde onun dosyasi kapali olacagindan
SIGPIPE sinyali verilecektir. Boyle bir durumda programin sorun olmadan calismaya devam etmesi
icin sigpipe handler yazilmistir. sigpipe handler globalde tutulan error flagini 1 yapar daha sonra
SIGPIPE sinyalini ignore eder. Daha sonra simpcat icinde error flagin 1 oldugu fark edilirse o
baglantinin artik aktif olmadigi anlasilir ve baglanti kontrol yapisindaki aktif alani 0 yapilir. Simpcat
sadece aktif olan baglantilara yazma yapacagindan bundan sonrasi icin olasi problemler ortadan
kaldirilmis olur.
Jtalk console mainden fork edilen bir thread'de calisir. Sunucudaki mevcut baglant durumlarini
izlemek ve sunucuyu kapatmak icin bir arayuz olusturur. Eger kullanici yanlis bir komut girerse HELP
mesajiyla kullaniciya dogru komutlar ve ne ise yaradiklari bildirilir. TALKERS komutu baglanti
agacini gezerek aktif olan baglantilari ve bu baglantilarin en son mesaj gonderdikleri zamani yazdirir.
ALL komutu serverdaki aktif olan ve olmayan tum baglantilari, ilk baglanma zamanlarini, cikis yapma
zamanlarini, ve en son mesaj attiklari zamani yazdirir. EXIT komutu ise serverin calismasini
sonlandirir. TALKERS ve ALL komutlari agacla ilgili islemlerini mutex'i kilitleyerek yaparlar.
