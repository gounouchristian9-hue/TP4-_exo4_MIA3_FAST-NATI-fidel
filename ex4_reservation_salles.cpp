// Exercice 4 - Systeme de reservation de salles
// Fichier unique, compatible C++11.
// Compilation : g++ -std=c++11 -Wall -Wextra -o reservation ex4_reservation_salles.cpp
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// ------------------------------------------------------------ Exception metier
class ReservationException : public std::runtime_error {
public:
    explicit ReservationException(const std::string& message) : std::runtime_error(message) {}
};

static std::string deuxChiffres(int n) {
    std::ostringstream o;
    o << std::setw(2) << std::setfill('0') << n;
    return o.str();
}

// ---------------------------------------------------------------- Categorie
enum class Categorie { COURS, TD, SOUTENANCE };

static std::string nomCategorie(Categorie c) {
    switch (c) {
    case Categorie::COURS: return "Cours";
    case Categorie::TD: return "TD";
    case Categorie::SOUTENANCE: return "Soutenance";
    }
    return "?";
}

// -------------------------------------------------------------------- Salle
class Salle {
    std::string numero_;
    int capacite_;
    Categorie categorie_;
public:
    Salle(const std::string& numero, int capacite, Categorie categorie)
        : numero_(numero), capacite_(capacite), categorie_(categorie) {
        if (numero_.empty()) throw ReservationException("Le numero de salle est obligatoire");
        if (capacite_ <= 0) throw ReservationException("La capacite doit etre strictement positive");
    }
    const std::string& numero() const { return numero_; }
    int capacite() const { return capacite_; }
    Categorie categorie() const { return categorie_; }
    std::string toString() const {
        return numero_ + " | " + nomCategorie(categorie_) + " | capacite " + std::to_string(capacite_);
    }
};

// --------------------------------------------------------------- Enseignant
class Enseignant {
    std::string id_, nom_, prenom_;
public:
    Enseignant(const std::string& id, const std::string& nom, const std::string& prenom)
        : id_(id), nom_(nom), prenom_(prenom) {
        if (id_.empty() || nom_.empty() || prenom_.empty())
            throw ReservationException("Identifiant, nom et prenom sont obligatoires");
    }
    const std::string& id() const { return id_; }
    std::string nomComplet() const { return prenom_ + " " + nom_; }
};

// --------------------------------------------------------------------- Date
class Date {
    int jour_, mois_, annee_;
    static bool bissextile(int a) { return (a % 4 == 0 && a % 100 != 0) || a % 400 == 0; }
    static int joursDansMois(int m, int a) {
        static const int j[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        return (m == 2 && bissextile(a)) ? 29 : j[m - 1];
    }
public:
    Date(int jour, int mois, int annee) : jour_(jour), mois_(mois), annee_(annee) {
        if (annee_ < 2000 || annee_ > 2100 || mois_ < 1 || mois_ > 12 || jour_ < 1 || jour_ > joursDansMois(mois_, annee_))
            throw ReservationException("Date invalide");
    }
    static Date depuisTexte(const std::string& s) {  // format JJ/MM/AAAA
        std::istringstream in(s);
        int j = 0, m = 0, a = 0;
        char c1 = 0, c2 = 0;
        if (!(in >> j >> c1 >> m >> c2 >> a) || c1 != '/' || c2 != '/')
            throw ReservationException("Format de date attendu : JJ/MM/AAAA");
        return Date(j, m, a);
    }
    bool operator==(const Date& o) const { return jour_ == o.jour_ && mois_ == o.mois_ && annee_ == o.annee_; }
    std::string toString() const { return deuxChiffres(jour_) + "/" + deuxChiffres(mois_) + "/" + std::to_string(annee_); }
};

// ------------------------------------------------------------------ Creneau
class Creneau {
    int debut_, fin_;  // en minutes depuis minuit
    static int minutes(const std::string& s) {  // format HH:MM
        std::istringstream in(s);
        int h = 0, m = 0;
        char c = 0;
        if (!(in >> h >> c >> m) || c != ':' || h < 0 || h > 24 || m < 0 || m > 59)
            throw ReservationException("Format d'heure attendu : HH:MM");
        return h * 60 + m;
    }
public:
    Creneau(int debut, int fin) : debut_(debut), fin_(fin) {
        if (debut_ < 0 || fin_ > 24 * 60 || debut_ >= fin_)
            throw ReservationException("Creneau invalide : l'heure de debut doit preceder l'heure de fin");
    }
    static Creneau depuisTexte(const std::string& debut, const std::string& fin) {
        return Creneau(minutes(debut), minutes(fin));
    }
    // Deux creneaux qui se touchent (10:00 fin / 10:00 debut) ne se chevauchent pas
    bool chevauche(const Creneau& o) const { return debut_ < o.fin_ && o.debut_ < fin_; }
    std::string toString() const {
        return deuxChiffres(debut_ / 60) + ":" + deuxChiffres(debut_ % 60) + "-" +
               deuxChiffres(fin_ / 60) + ":" + deuxChiffres(fin_ % 60);
    }
};

// -------------------------------------------------------------- Reservation
class Reservation {
    int id_;
    std::string numeroSalle_, idEnseignant_, motif_;
    Date date_;
    Creneau creneau_;
    int participants_;
    bool active_;
public:
    Reservation(int id, const std::string& numeroSalle, const std::string& idEnseignant, const Date& date,
                const Creneau& creneau, int participants, const std::string& motif)
        : id_(id), numeroSalle_(numeroSalle), idEnseignant_(idEnseignant), motif_(motif),
          date_(date), creneau_(creneau), participants_(participants), active_(true) {}
    int id() const { return id_; }
    const std::string& numeroSalle() const { return numeroSalle_; }
    const std::string& idEnseignant() const { return idEnseignant_; }
    const Date& date() const { return date_; }
    const Creneau& creneau() const { return creneau_; }
    bool estActive() const { return active_; }
    void annuler() {
        if (!active_) throw ReservationException("Reservation deja annulee");
        active_ = false;
    }
    std::string toString() const {
        return "R" + std::to_string(id_) + " | salle " + numeroSalle_ + " | " + idEnseignant_ + " | " +
               date_.toString() + " " + creneau_.toString() + " | " + std::to_string(participants_) +
               " pers. | " + motif_ + " | " + (active_ ? "ACTIVE" : "ANNULEE");
    }
};

// ------------------------------------------------ Gestionnaire de reservations
class GestionnaireReservations {
    std::vector<Salle> salles_;
    std::vector<Enseignant> enseignants_;
    std::vector<Reservation> reservations_;
    int prochainId_;

    const Salle* chercherSalle(const std::string& numero) const {
        for (const auto& s : salles_) if (s.numero() == numero) return &s;
        return nullptr;
    }
    const Enseignant* chercherEnseignant(const std::string& id) const {
        for (const auto& e : enseignants_) if (e.id() == id) return &e;
        return nullptr;
    }
    // Reservation active qui entre en conflit avec (salle, date, creneau), ou nullptr
    const Reservation* conflit(const std::string& numero, const Date& date, const Creneau& cr) const {
        for (const auto& r : reservations_)
            if (r.estActive() && r.numeroSalle() == numero && r.date() == date && r.creneau().chevauche(cr))
                return &r;
        return nullptr;
    }
public:
    GestionnaireReservations() : prochainId_(1) {}

    const std::vector<Salle>& salles() const { return salles_; }
    const std::vector<Reservation>& reservations() const { return reservations_; }

    void ajouterSalle(const Salle& s) {
        if (chercherSalle(s.numero())) throw ReservationException("La salle " + s.numero() + " existe deja");
        salles_.push_back(s);
    }
    void ajouterEnseignant(const Enseignant& e) {
        if (chercherEnseignant(e.id())) throw ReservationException("L'enseignant " + e.id() + " existe deja");
        enseignants_.push_back(e);
    }

    bool estDisponible(const std::string& numero, const Date& date, const Creneau& cr) const {
        if (!chercherSalle(numero)) throw ReservationException("Salle inconnue : " + numero);
        return conflit(numero, date, cr) == nullptr;
    }

    std::vector<Salle> sallesDisponibles(const Date& date, const Creneau& cr, int effectif) const {
        std::vector<Salle> r;
        for (const auto& s : salles_)
            if (s.capacite() >= effectif && conflit(s.numero(), date, cr) == nullptr) r.push_back(s);
        return r;
    }

    // Enregistre une reservation et retourne son identifiant
    int reserver(const std::string& idEnseignant, const std::string& numero, const Date& date,
                 const Creneau& cr, int participants, const std::string& motif) {
        const Enseignant* e = chercherEnseignant(idEnseignant);
        if (!e) throw ReservationException("Enseignant inconnu : " + idEnseignant);
        const Salle* s = chercherSalle(numero);
        if (!s) throw ReservationException("Salle inconnue : " + numero);
        if (participants <= 0) throw ReservationException("Le nombre de participants doit etre positif");
        if (participants > s->capacite())
            throw ReservationException("Capacite depassee : " + std::to_string(participants) + " participants pour " +
                                       std::to_string(s->capacite()) + " places");
        const Reservation* c = conflit(numero, date, cr);
        if (c)
            throw ReservationException("Conflit : la salle " + numero + " est deja reservee le " + date.toString() +
                                       " (" + c->creneau().toString() + ") par " + c->idEnseignant());
        reservations_.push_back(Reservation(prochainId_, numero, idEnseignant, date, cr, participants, motif));
        return prochainId_++;
    }

    // Seul l'enseignant qui a effectue la reservation peut l'annuler
    void annuler(int idReservation, const std::string& idEnseignant) {
        for (auto& r : reservations_) {
            if (r.id() != idReservation) continue;
            if (r.idEnseignant() != idEnseignant)
                throw ReservationException("Annulation refusee : cette reservation appartient a " + r.idEnseignant());
            r.annuler();
            return;
        }
        throw ReservationException("Reservation inconnue : R" + std::to_string(idReservation));
    }

    std::vector<Reservation> reservationsDeSalle(const std::string& numero) const {
        std::vector<Reservation> r;
        for (const auto& x : reservations_) if (x.numeroSalle() == numero && x.estActive()) r.push_back(x);
        return r;
    }
    std::vector<Reservation> reservationsDeEnseignant(const std::string& id) const {
        std::vector<Reservation> r;
        for (const auto& x : reservations_) if (x.idEnseignant() == id && x.estActive()) r.push_back(x);
        return r;
    }
};

// ----------------------------------------------------------------------- IHM
static std::string lireTexte(const std::string& question) {
    std::cout << question << " : ";
    std::string s;
    if (!std::getline(std::cin, s)) throw std::runtime_error("Fin de saisie");
    if (s.empty()) throw ReservationException("Valeur obligatoire");
    return s;
}
static int lireEntier(const std::string& question) {
    try { return std::stoi(lireTexte(question)); }
    catch (const std::invalid_argument&) { throw ReservationException("Nombre invalide"); }
    catch (const std::out_of_range&) { throw ReservationException("Nombre invalide"); }
}
static Categorie lireCategorie() {
    int c = lireEntier("Categorie (1=Cours, 2=TD, 3=Soutenance)");
    if (c == 1) return Categorie::COURS;
    if (c == 2) return Categorie::TD;
    if (c == 3) return Categorie::SOUTENANCE;
    throw ReservationException("Categorie invalide");
}
static Creneau lireCreneau() {
    std::string debut = lireTexte("Heure de debut (HH:MM)");
    std::string fin = lireTexte("Heure de fin (HH:MM)");
    return Creneau::depuisTexte(debut, fin);
}

static void donneesDemo(GestionnaireReservations& g) {
    g.ajouterSalle(Salle("A101", 60, Categorie::COURS));
    g.ajouterSalle(Salle("TD02", 30, Categorie::TD));
    g.ajouterSalle(Salle("S1", 20, Categorie::SOUTENANCE));
    g.ajouterEnseignant(Enseignant("ENS1", "Kone", "Awa"));
    g.ajouterEnseignant(Enseignant("ENS2", "Diallo", "Moussa"));
    g.reserver("ENS1", "A101", Date(15, 12, 2026), Creneau(8 * 60, 10 * 60), 45, "Cours POO");
}

static void menu() {
    std::cout << "\n===== Reservation de salles =====\n"
                 " 1. Ajouter une salle              2. Ajouter un enseignant\n"
                 " 3. Lister les salles              4. Verifier la disponibilite d'une salle\n"
                 " 5. Salles disponibles (creneau)   6. Reserver une salle\n"
                 " 7. Annuler une reservation        8. Lister toutes les reservations\n"
                 " 9. Reservations d'une salle      10. Charger des donnees de demonstration\n"
                 " 0. Quitter\n";
}

int main() {
    GestionnaireReservations g;
    while (true) {
        menu();
        try {
            int choix = lireEntier("Choix");
            switch (choix) {
            case 0: std::cout << "Au revoir.\n"; return 0;
            case 1: {
                std::string n = lireTexte("Numero");
                int cap = lireEntier("Capacite");
                g.ajouterSalle(Salle(n, cap, lireCategorie()));
                break;
            }
            case 2: {
                std::string id = lireTexte("Identifiant"), nom = lireTexte("Nom"), pre = lireTexte("Prenom");
                g.ajouterEnseignant(Enseignant(id, nom, pre));
                break;
            }
            case 3:
                for (const auto& s : g.salles()) std::cout << s.toString() << "\n";
                break;
            case 4: {
                std::string n = lireTexte("Numero de salle");
                Date d = Date::depuisTexte(lireTexte("Date (JJ/MM/AAAA)"));
                Creneau c = lireCreneau();
                std::cout << "Salle " << n << (g.estDisponible(n, d, c) ? " DISPONIBLE" : " INDISPONIBLE")
                          << " le " << d.toString() << " " << c.toString() << "\n";
                break;
            }
            case 5: {
                Date d = Date::depuisTexte(lireTexte("Date (JJ/MM/AAAA)"));
                Creneau c = lireCreneau();
                int eff = lireEntier("Effectif minimal");
                std::vector<Salle> v = g.sallesDisponibles(d, c, eff);
                if (v.empty()) std::cout << "Aucune salle disponible.\n";
                for (const auto& s : v) std::cout << s.toString() << "\n";
                break;
            }
            case 6: {
                std::string ens = lireTexte("Identifiant enseignant"), n = lireTexte("Numero de salle");
                Date d = Date::depuisTexte(lireTexte("Date (JJ/MM/AAAA)"));
                Creneau c = lireCreneau();
                int nb = lireEntier("Nombre de participants");
                std::string motif = lireTexte("Motif");
                std::cout << "Reservation R" << g.reserver(ens, n, d, c, nb, motif) << " enregistree.\n";
                break;
            }
            case 7: {
                int id = lireEntier("Numero de reservation (sans le R)");
                std::string ens = lireTexte("Identifiant enseignant");
                g.annuler(id, ens);
                std::cout << "Reservation R" << id << " annulee.\n";
                break;
            }
            case 8:
                for (const auto& r : g.reservations()) std::cout << r.toString() << "\n";
                break;
            case 9: {
                std::string n = lireTexte("Numero de salle");
                for (const auto& r : g.reservationsDeSalle(n)) std::cout << r.toString() << "\n";
                break;
            }
            case 10: donneesDemo(g); std::cout << "Donnees de demonstration chargees.\n"; break;
            default: std::cout << "Choix invalide.\n";
            }
        } catch (const std::exception& e) {
            if (std::cin.eof()) { std::cout << "\nFin de saisie.\n"; return 0; }
            std::cout << "Erreur : " << e.what() << "\n";
        }
    }
}
