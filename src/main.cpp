/*
KROTKA INSTRUCKJA GRY

Aby wygrac ponizsza gre, trzeba spelnic dwa warunki- przekroczyc wartosc firmy 50 000 PLN i miec
splacone wszystkie dlugi. Startuje sie z budzetem 10 000 PLN i wartoscia firmy 10 000 PLN (nie sa
to te same rzeczy).
Przy braniu kredytu, nalezy najpierw wpisac kred, dopiero potem pojawia sie dalsze opcje pozyczki.
To samo tyczy sie splaty dodatkowej. Polecenia budzet i wartosc sa pomocnicze, uzywalem ich zeby sie
nie pogubic przy wczesniejszych wersjach gry, kiedy ta nie byla taka latwa do wygrania. Teraz nie powinno
to byc problemem :).
SPOJLER JAK WYGRAC
Jakby jednak pojawil sie takowy, polecam na start wziac kredyt na 25 000 PLN i zatrudnic robotnika i inzyniera.
W nastepnej turze magazyniera i marketera, a potem to jakos pojdzie. Mam nadzieje ze w ten sposob nie zepsulem zabawy
z uzywania programu.
*/

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <random>
#include <iomanip>
#include <algorithm>

// Stale wydajnosci pracownikow
constexpr int CI = 200;    // Cena produktu na inzyniera
constexpr int CMag = 50;   // Pojemnosc magazynu na magazyniera
constexpr int CMkt = 60;   // Popyt na produkty na marketera
constexpr int CR = 40;     // Produkcja produktów na robotnika

// Inne stale
constexpr int MAX_CREDIT_DURATION = 36; // Maksymalny czas splaty kredytu w miesiacach
constexpr double MAX_DEBT_MULTIPLIER = 3.0; // Maksymalne zadluzenie jako wielokrotnosc wartosci firmy
constexpr int VALUE_HISTORY = 6; // Liczba miesiecy do obliczenia sredniego przychodu

//Klasa bazowa pracownikow
struct pracownik {
    std::string imie;
    double wynagrodzenie;

    pracownik(const std::string& imie, double wynagrodzenie)
        : imie(imie), wynagrodzenie(wynagrodzenie) {}

    virtual ~pracownik() = default;
    virtual void print() const = 0;
};

//Podklasy pracownikow
struct inzynier : public pracownik {
    std::string wydzial;

    inzynier(const std::string& imie, double wynagrodzenie, const std::string& wydzial)
        : pracownik(imie, wynagrodzenie), wydzial(wydzial) {}

    void print() const override {
        std::cout <<"Inzynier " <<imie << ", wydzial " <<wydzial << ", wynagrodzenie " <<wynagrodzenie << "\n";
    }
};

struct magazynier : public pracownik {
    bool obsluga_widlowego;

    magazynier(const std::string& imie, double wynagrodzenie, bool obsluga_widlowego)
        : pracownik(imie, wynagrodzenie), obsluga_widlowego(obsluga_widlowego) {}

    void print() const override {
        std::cout << "Magazynier " << imie << ", obsluga widlaka: " << (obsluga_widlowego ? "Tak" : "Nie") << ", wynagrodzenie: " << wynagrodzenie << "\n";
    }
};

struct marketer : public pracownik {
    int follows;

    marketer(const std::string& imie, double wynagrodzenie, int follows)
        : pracownik(imie, wynagrodzenie), follows(follows) {}

    void print() const override {
        std::cout << "Marketer " << imie << ", obserwujacy: " << follows << ", wynagrodzenie: " << wynagrodzenie << "\n";
    }
};

struct robotnik : public pracownik {
    double rozmiar_buta;

    robotnik(const std::string& imie, double wynagrodzenie, double rozmiar_buta)
        : pracownik(imie, wynagrodzenie), rozmiar_buta(rozmiar_buta) {}

    void print() const override {
        std::cout << "Robotnik " << imie << ", rozmiar buta: " << rozmiar_buta << ", wynagrodzenie: " << wynagrodzenie << "\n";
    }
};

//Klasa kredytu
struct kredyt {
    double dlug;
    int pozostale_raty;

    double splac_rate() const {         //oblicza kwote comiesiecznej raty do splacenia
        return dlug / pozostale_raty;
    }
};

//Klasa firmy
class Firma {
    double stan_konta;
    int n_kredytow = 0;
    double wartosc_poczatkowa = 10000.0;
    double wydatki_biezace = 0.0;
    std::vector < std::unique_ptr < kredyt > > kredyty;
    std::vector < std::variant<inzynier,magazynier,marketer,robotnik> > pracownicy;
    std::vector < double > historia_przychodow;

    public:
    Firma() : stan_konta(10000.0) {
        pracownicy.emplace_back(inzynier("Jan", 4000.0, "MEiL"));
        pracownicy.emplace_back(magazynier("Adam", 3000.0, true));
        pracownicy.emplace_back(marketer("Anna", 3500.0, 1500));
        pracownicy.emplace_back(robotnik("Ewa", 3200.0, 42.0));
    }

    void drukuj_pracownikow() const {
        for (const auto& pracownik : pracownicy) {
            std::visit([](const auto& p) { p.print(); }, pracownik);
        }
    }

    void zatrudnij(const std::variant<inzynier, magazynier, marketer, robotnik>& nowy) {
        pracownicy.push_back(nowy);
    }

    void wez_kredyt(double kwota, int czas_splaty) {
    if (czas_splaty > MAX_CREDIT_DURATION) {
        std::cout << "Maksymalny czas splaty to " << MAX_CREDIT_DURATION << " miesiecy.\n";
        return;
    }
    if (kwota + zadluzenie() > MAX_DEBT_MULTIPLIER * wartosc_firmy()) {
        std::cout << "Nie mozna przekroczyc maksymalnego zadluzenia.\n";
        return;
    }

    // Obliczenie odsetek
    double odsetki = 1.0 + 0.05 * czas_splaty;
    double calkowity_dlug = kwota * odsetki;

    // Dodanie kredytu do listy
    kredyty.push_back(std::make_unique<kredyt>(kredyt{calkowity_dlug, czas_splaty}));

    // Zwiekszenie stanu konta
    stan_konta += kwota;

    // Zwiekszenie licznika kredytow
    n_kredytow++;

    // Informacja o wzietym kredycie
    std::cout << "Wzieto kredyt na kwote " << kwota << " PLN na " << czas_splaty << " miesiecy.\n";
    std::cout << "Stan konta po wzieciu kredytu: " << stan_konta << " PLN\n";
}

    void zaplac_wynagrodzenie() {
        double suma = 0.0;
        for (const auto& pracownik : pracownicy) {
            std::visit([&suma](const auto& p) { suma += p.wynagrodzenie; }, pracownik);
        }
        stan_konta -= suma;
        wydatki_biezace += suma;
    }

    void otrzymaj_przychod() {
    double przychod = oblicz_przychod();
    historia_przychodow.push_back(przychod);
    if (historia_przychodow.size() > VALUE_HISTORY) {
        historia_przychodow.erase(historia_przychodow.begin());
    }
    stan_konta += przychod;

    }

    void splac_raty() {
    double suma_rat = 0.0;

    for (auto& kredyt : kredyty) {
        if (kredyt) {                                            // Upewniam sie, ze kredyt nie jest pusty
            double rata = kredyt->dlug / kredyt->pozostale_raty; // Obliczanie raty
            if (stan_konta >= rata) {
                stan_konta -= rata;
                kredyt->dlug -= rata;                            // Zmniejszanie zadluzenia
                suma_rat += rata;
                kredyt->pozostale_raty--;

                // Usuwamm kredyt, jesli zadluzenie zostalo splacone
                if (kredyt->dlug <= 0 || kredyt->pozostale_raty <= 0) {
                    kredyt.reset(); // Usuwam kredyt
                }
            } else {
                std::cout << "Brak wystarczajacych srodkow na splate kredytu w tym miesiacu!\n";
            }
        }
    }

    // Usuwanie splaconych kredytow
    kredyty.erase(std::remove(kredyty.begin(), kredyty.end(), nullptr), kredyty.end());

    std::cout << "Splacono raty kredytow w wysokosci: " << suma_rat << " PLN\n";
    }

    void dodatkowa_splata(double kwota) {
    if (kwota > stan_konta) {
        std::cout << "Nie masz wystarczajacych srodkow na dodatkowa splate kredytu!\n";
        return;
    }

    double suma_splat = 0.0;

    for (auto& kredyt : kredyty) {
        if (kwota <= 0) break;                            // Jesli wszystko splacone, przerywa

        double do_splaty = std::min(kwota, kredyt->dlug); // Kwota do splaty w biezacym kredycie
        kredyt->dlug -= do_splaty;                        // Zmniejsza dlug kredytu
        suma_splat += do_splaty;
        kwota -= do_splaty;

        // Jesli kredyt zostal splacony, usuwa go
        if (kredyt->dlug <= 0) {
            kredyt.reset();
        }
    }

    // Usuwanie splaconych kredytow
    kredyty.erase(std::remove(kredyty.begin(), kredyty.end(), nullptr), kredyty.end());

    stan_konta -= suma_splat; // Redukuje saldo o cala sume splaty
    std::cout << "Dodatkowo splacono kredyty na kwote: " << suma_splat << " PLN\n";
    }

    double oblicz_przychod() const {
    int liczba_inz = 0, liczba_mag = 0, liczba_mkt = 0, liczba_rob = 0;

    // Oblicza liczby pracownikow roznych typow
    for (const auto& pracownik : pracownicy) {
        std::visit([&](const auto& p) {
            using T = std::decay_t<decltype(p)>;
            if constexpr (std::is_same_v<T, inzynier>) liczba_inz++;
            if constexpr (std::is_same_v<T, magazynier>) liczba_mag++;
            if constexpr (std::is_same_v<T, marketer>) liczba_mkt++;
            if constexpr (std::is_same_v<T, robotnik>) liczba_rob++;
        }, pracownik);
    }

    // Pojemnosc magazynu
    int pojemnosc_mag = liczba_mag * CMag;

    // Cena produktu
    int cena_produktu = liczba_inz * CI;

    // Popyt
    int popyt = liczba_mkt * CMkt;

    // Produkcja i sprzedaz
    int teoretyczna_produkcja = liczba_rob * CR;
    int faktyczna_produkcja = std::min(teoretyczna_produkcja, pojemnosc_mag);
    int sprzedaz = std::min(faktyczna_produkcja, popyt);

    // Przychod
    return sprzedaz * cena_produktu;
}

     double wartosc_firmy() const {
        if (historia_przychodow.empty()) return wartosc_poczatkowa;
        double suma = 0.0;
        for (double przychod : historia_przychodow) suma += przychod;
        return suma / historia_przychodow.size();
    }

    double zadluzenie() const {
        double suma = 0.0;
        for (const auto& kredyt : kredyty) suma += kredyt->dlug;
        return suma;
    }

    double get_stan_konta() const {
        return stan_konta;
    }

    void stan_firmy() const {
    std::cout << "Stan konta: " << stan_konta << " PLN\n";
    std::cout << "Wartosc firmy: " << wartosc_firmy() << " PLN\n";
    std::cout << "Zadluzenie: " << zadluzenie() << " PLN\n";
    std::cout << "Liczba kredytow: " << n_kredytow << "\n";
    std::cout << "Wydatki w tym miesiacu: " << wydatki_biezace << " PLN\n";

    if (!historia_przychodow.empty()) {
        std::cout << "Ostatni przychod: " << historia_przychodow.back() << " PLN\n";
    } else {
        std::cout << "Brak przychodu w historii.\n";
    }
    }

    void drukuj_informacje_kredytowe() const {
    double laczna_kwota_rat = 0.0;
    int pozostale_miesiace = 0;

    for (const auto& kredyt : kredyty) {
        if (kredyt) {
            laczna_kwota_rat += kredyt->splac_rate() * kredyt->pozostale_raty;
            pozostale_miesiace += kredyt->pozostale_raty;
        }
    }

    std::cout << "Laczna kwota pozostala do splaty kredytow: " << laczna_kwota_rat << " PLN\n";
    std::cout << "Laczna liczba pozostalych miesiecy splaty: " << pozostale_miesiace << " miesiecy\n";
    }

    bool czy_kredyty_splacone() const {
    return std::all_of(kredyty.begin(), kredyty.end(), [](const std::unique_ptr<kredyt>& k) {
        return k->dlug <= 0;
    });
    }

    bool czy_niesplacony_kredyt_przeterminowany() const {
    return std::any_of(kredyty.begin(), kredyty.end(), [](const std::unique_ptr<kredyt>& k) {
        return k->pozostale_raty <= 0 && k->dlug > 0;
    });
    }

    void resetuj_wydatki() {
    wydatki_biezace = 0.0;
    }
    
};

//klasa gry
class gra {
    private:
        Firma firma;
        bool stan = true;

    public:
        void akcja_gracza() {
        std::cout << "Dostepne akcje:\n"
                  << "lp - Wylistuj pracownikow\n"
                  << "zinz - Zatrudnij inżyniera\n"
                  << "zmag - Zatrudnij magazyniera\n"
                  << "zmkt - Zatrudnij marketera\n"
                  << "zrob - Zatrudnij robotnika\n"
                  << "kred - Wez kredyt\n"
                  <<"splata - Dodatkowa splata kredytu\n"
                  << "budzet - Wyswietl budzet firmy\n"
                  << "wartosc - Wyswietl wartosc firmy\n"
                  << "kt - Zakoncz ture\n";
        std::string akcja;
        std::cin >> akcja;

        if (akcja == "lp") {
            firma.drukuj_pracownikow();
        } else if (akcja == "zinz") {
            firma.zatrudnij(inzynier("Inz_" + std::to_string(rand() % 100), 4000.0, "Elektryczny"));
        } else if (akcja == "zmag") {
            firma.zatrudnij(magazynier("Mag_" + std::to_string(rand() % 100), 3000.0, rand() % 2)); // losowo 0 lub 1, zeby ocenic czy moze prowadzic widlaka
        } else if (akcja == "zmkt") {
            firma.zatrudnij(marketer("Mkt_" + std::to_string(rand() % 100), 3500.0, rand() % 10000));
        } else if (akcja == "zrob") {
            firma.zatrudnij(robotnik("Rob_" + std::to_string(rand() % 100), 3200.0, rand() % 50));
        } else if (akcja == "kred") {
            double kwota;
            int czas;
            std::cout << "Podaj kwote kredytu: ";
            std::cin >> kwota;
            std::cout << "Podaj czas splaty (miesiace): ";
            std::cin >> czas;
            firma.wez_kredyt(kwota, czas);
        } else if (akcja == "splata") {
        double kwota;
        std::cout << "Podaj kwote do dodatkowej splaty kredytu: ";
        std::cin >> kwota;
        firma.dodatkowa_splata(kwota);
        } else if (akcja == "budzet") {
            std::cout << "Budzet firmy: " << firma.get_stan_konta() << " PLN\n";
        } else if (akcja == "wartosc") {
            std::cout << "Wartosc firmy: " << firma.wartosc_firmy() << " PLN\n";
        }else if (akcja == "kt") {
            firma.splac_raty();
            firma.zaplac_wynagrodzenie();
            firma.otrzymaj_przychod();
            firma.stan_firmy();
            firma.drukuj_informacje_kredytowe();
            firma.resetuj_wydatki();
        } else {
            std::cout << "Nieznana akcja. Sprobuj ponownie.\n";
        }
    }

    bool get_stan() const {
        return stan;
    }

    void tick() {
    // Sprawdzenie zadluzenia
        if (firma.czy_niesplacony_kredyt_przeterminowany()) {
            std::cout << "Twoj kredyt nie został splacony na czas. Przegrales!\n";
            stan = false;
            return;
        }

    // Sprawdzenie stanu konta
        if (firma.get_stan_konta() < 0) {
            std::cout << "Twoja firma zbankrutowala!\n";
            stan = false;
            return;
        }

    // Sprawdzenie wygranej
        if (firma.wartosc_firmy() >= 50000.0 && firma.czy_kredyty_splacone()) {
            std::cout << "Gratulacje! Osiągnales wartosc firmy 50 000 PLN i splaciles wszystkie kredyty. Wygrales!\n";
            stan = false;
        }
    }

};

int main()
{
    bool chce_zagrac_ponownie = true;

    while (chce_zagrac_ponownie) {
        gra gra;
        std::cout << "Witaj w grze ekonomicznej! \nNa start Twoja firma ma wartosc 10 000 PLN oraz operuje budzetem 10 000 PLN.\nTwoim celem jest osiagniecie wartosci firmy 50 000 PLN.\n";

        while (gra.get_stan()) {
            gra.akcja_gracza();
            gra.tick();
        }

        std::cout << "Czy chcesz zagrac jeszcze raz? (tak/nie): ";
        std::string odpowiedz;
        std::cin >> odpowiedz;

        if (odpowiedz == "nie" || odpowiedz == "n") {
            chce_zagrac_ponownie = false;
            std::cout << "Dzieki za gre!\n";
        } else if (odpowiedz == "tak" || odpowiedz == "t") {
            std::cout << "Rozpoczynamy nowa gre!\n";
        } else {
            std::cout << "Niepoprawna odpowiedz. Zakladam, ze nie chcesz grac dalej.\n";
            chce_zagrac_ponownie = false;
        }
    }

    return 0;
}