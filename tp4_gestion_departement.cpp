// TP 4 - Systeme integre de gestion d'un departement universitaire
// Fichier unique, compatible C++11 (aucune dependance a std::filesystem).
// Compilation : g++ -std=c++11 -Wall -Wextra -o gestion_departement tp4_gestion_departement.cpp
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef _WIN32
#include <direct.h>
static void creerDossier(const std::string& d) { _mkdir(d.c_str()); }
#else
#include <sys/stat.h>
static void creerDossier(const std::string& d) { mkdir(d.c_str(), 0755); }
#endif

// Classes métier du TP 4 : gestion d'un département universitaire

inline std::vector<std::string> decouper(const std::string& s, char d = ';') {
    std::vector<std::string> r;
    std::string t;
    std::stringstream ss(s);
    while (std::getline(ss, t, d)) r.push_back(t);
    return r;
}

// Découpe une ligne CSV et garantit exactement n champs
inline std::vector<std::string> champs(const std::string& ligne, size_t n) {
    auto v = decouper(ligne);
    while (v.size() < n) v.push_back("");
    if (v.size() > n) throw std::runtime_error("Ligne CSV invalide : " + ligne);
    return v;
}

// ---------------------------------------------------------------- Personnes
class Personne {
protected:
    std::string id_, nom_, prenom_;
public:
    Personne(std::string id, std::string nom, std::string prenom)
        : id_(std::move(id)), nom_(std::move(nom)), prenom_(std::move(prenom)) {}
    virtual ~Personne() = default;
    const std::string& id() const { return id_; }
    const std::string& nom() const { return nom_; }
    const std::string& prenom() const { return prenom_; }
    std::string nomComplet() const { return prenom_ + " " + nom_; }
    virtual std::string toCsv() const = 0;
};

class Etudiant : public Personne {
    std::string filiere_, niveau_;
public:
    Etudiant(std::string id, std::string nom, std::string prenom, std::string filiere, std::string niveau)
        : Personne(std::move(id), std::move(nom), std::move(prenom)),
          filiere_(std::move(filiere)), niveau_(std::move(niveau)) {}
    const std::string& filiere() const { return filiere_; }
    const std::string& niveau() const { return niveau_; }
    std::string toCsv() const override {
        return id_ + ";" + nom_ + ";" + prenom_ + ";" + filiere_ + ";" + niveau_;
    }
    static Etudiant fromCsv(const std::string& l) {
        auto c = champs(l, 5);
        return Etudiant(c[0], c[1], c[2], c[3], c[4]);
    }
};

class Enseignant : public Personne {
    std::string specialite_;
public:
    Enseignant(std::string id, std::string nom, std::string prenom, std::string specialite)
        : Personne(std::move(id), std::move(nom), std::move(prenom)), specialite_(std::move(specialite)) {}
    const std::string& specialite() const { return specialite_; }
    std::string toCsv() const override {
        return id_ + ";" + nom_ + ";" + prenom_ + ";" + specialite_;
    }
    static Enseignant fromCsv(const std::string& l) {
        auto c = champs(l, 4);
        return Enseignant(c[0], c[1], c[2], c[3]);
    }
};

// ------------------------------------------------------ Structure pédagogique
class Filiere {
    std::string code_, nom_;
public:
    Filiere(std::string code, std::string nom) : code_(std::move(code)), nom_(std::move(nom)) {}
    const std::string& code() const { return code_; }
    const std::string& nom() const { return nom_; }
    std::string toCsv() const { return code_ + ";" + nom_; }
    static Filiere fromCsv(const std::string& l) { auto c = champs(l, 2); return Filiere(c[0], c[1]); }
};

class Niveau {
    std::string code_, libelle_;
public:
    Niveau(std::string code, std::string libelle) : code_(std::move(code)), libelle_(std::move(libelle)) {}
    const std::string& code() const { return code_; }
    const std::string& libelle() const { return libelle_; }
    std::string toCsv() const { return code_ + ";" + libelle_; }
    static Niveau fromCsv(const std::string& l) { auto c = champs(l, 2); return Niveau(c[0], c[1]); }
};

class UE {
    std::string code_, intitule_, filiere_, niveau_;
    int credits_;
public:
    UE(std::string code, std::string intitule, int credits, std::string filiere, std::string niveau)
        : code_(std::move(code)), intitule_(std::move(intitule)), filiere_(std::move(filiere)),
          niveau_(std::move(niveau)), credits_(credits) {}
    const std::string& code() const { return code_; }
    const std::string& intitule() const { return intitule_; }
    const std::string& filiere() const { return filiere_; }
    const std::string& niveau() const { return niveau_; }
    int credits() const { return credits_; }
    std::string toCsv() const {
        return code_ + ";" + intitule_ + ";" + std::to_string(credits_) + ";" + filiere_ + ";" + niveau_;
    }
    static UE fromCsv(const std::string& l) {
        auto c = champs(l, 5);
        return UE(c[0], c[1], std::stoi(c[2]), c[3], c[4]);
    }
};

class Matiere {
    std::string code_, intitule_, codeUE_, idEnseignant_;
    double coefficient_;
public:
    Matiere(std::string code, std::string intitule, double coef, std::string codeUE, std::string idEns)
        : code_(std::move(code)), intitule_(std::move(intitule)), codeUE_(std::move(codeUE)),
          idEnseignant_(std::move(idEns)), coefficient_(coef) {}
    const std::string& code() const { return code_; }
    const std::string& intitule() const { return intitule_; }
    const std::string& codeUE() const { return codeUE_; }
    const std::string& idEnseignant() const { return idEnseignant_; }
    double coefficient() const { return coefficient_; }
    std::string toCsv() const {
        return code_ + ";" + intitule_ + ";" + std::to_string(coefficient_) + ";" + codeUE_ + ";" + idEnseignant_;
    }
    static Matiere fromCsv(const std::string& l) {
        auto c = champs(l, 5);
        return Matiere(c[0], c[1], std::stod(c[2]), c[3], c[4]);
    }
};

// ------------------------------------------------------------------- Notes
class Note {
    std::string idEtudiant_, codeMatiere_;
    double valeur_;
public:
    Note(std::string idEtu, std::string codeMat, double valeur)
        : idEtudiant_(std::move(idEtu)), codeMatiere_(std::move(codeMat)), valeur_(valeur) {}
    const std::string& idEtudiant() const { return idEtudiant_; }
    const std::string& codeMatiere() const { return codeMatiere_; }
    double valeur() const { return valeur_; }
    void setValeur(double v) { valeur_ = v; }
    std::string toCsv() const { return idEtudiant_ + ";" + codeMatiere_ + ";" + std::to_string(valeur_); }
    static Note fromCsv(const std::string& l) {
        auto c = champs(l, 3);
        return Note(c[0], c[1], std::stod(c[2]));
    }
};

// ------------------------------------------------------------ Salles et EDT
class Salle {
    std::string code_;
    int capacite_;
public:
    Salle(std::string code, int capacite) : code_(std::move(code)), capacite_(capacite) {}
    const std::string& code() const { return code_; }
    int capacite() const { return capacite_; }
    std::string toCsv() const { return code_ + ";" + std::to_string(capacite_); }
    static Salle fromCsv(const std::string& l) { auto c = champs(l, 2); return Salle(c[0], std::stoi(c[1])); }
};

class Seance {
    std::string codeMatiere_, idEnseignant_, codeSalle_, filiere_, niveau_;
    int jour_, debut_, fin_;   // jour : 1 = lundi ... 6 = samedi ; heures entières
public:
    Seance(std::string codeMat, std::string idEns, std::string codeSalle, std::string filiere,
           std::string niveau, int jour, int debut, int fin)
        : codeMatiere_(std::move(codeMat)), idEnseignant_(std::move(idEns)), codeSalle_(std::move(codeSalle)),
          filiere_(std::move(filiere)), niveau_(std::move(niveau)), jour_(jour), debut_(debut), fin_(fin) {}
    const std::string& codeMatiere() const { return codeMatiere_; }
    const std::string& idEnseignant() const { return idEnseignant_; }
    const std::string& codeSalle() const { return codeSalle_; }
    const std::string& filiere() const { return filiere_; }
    const std::string& niveau() const { return niveau_; }
    int jour() const { return jour_; }
    int debut() const { return debut_; }
    int fin() const { return fin_; }
    bool chevauche(const Seance& o) const { return jour_ == o.jour_ && debut_ < o.fin_ && o.debut_ < fin_; }
    std::string toCsv() const {
        return codeMatiere_ + ";" + idEnseignant_ + ";" + codeSalle_ + ";" + filiere_ + ";" + niveau_ + ";" +
               std::to_string(jour_) + ";" + std::to_string(debut_) + ";" + std::to_string(fin_);
    }
    static Seance fromCsv(const std::string& l) {
        auto c = champs(l, 8);
        return Seance(c[0], c[1], c[2], c[3], c[4], std::stoi(c[5]), std::stoi(c[6]), std::stoi(c[7]));
    }
};

class EmploiDuTemps {
    std::vector<Seance> seances_;
public:
    // Retourne le motif du conflit, ou "" s'il n'y en a pas
    std::string conflit(const Seance& s) const {
        for (const auto& o : seances_) {
            if (!s.chevauche(o)) continue;
            if (o.codeSalle() == s.codeSalle()) return "salle " + s.codeSalle() + " déjà occupée";
            if (o.idEnseignant() == s.idEnseignant()) return "enseignant " + s.idEnseignant() + " déjà occupé";
            if (o.filiere() == s.filiere() && o.niveau() == s.niveau()) return "groupe " + s.filiere() + "/" + s.niveau() + " déjà en cours";
        }
        return "";
    }
    void ajouter(const Seance& s) {
        std::string c = conflit(s);
        if (!c.empty()) throw std::runtime_error("Conflit d'emploi du temps : " + c);
        seances_.push_back(s);
    }
    std::vector<Seance> pour(const std::string& filiere, const std::string& niveau) const {
        std::vector<Seance> r;
        for (const auto& s : seances_) if (s.filiere() == filiere && s.niveau() == niveau) r.push_back(s);
        std::sort(r.begin(), r.end(), [](const Seance& a, const Seance& b) {
            return a.jour() != b.jour() ? a.jour() < b.jour() : a.debut() < b.debut();
        });
        return r;
    }
    const std::vector<Seance>& toutes() const { return seances_; }
};

// Service principal : règles de gestion, calculs académiques, relevés, persistance CSV

class Departement {
    std::string nom_;
    std::vector<Filiere> filieres_;
    std::vector<Niveau> niveaux_;
    std::vector<Enseignant> enseignants_;
    std::vector<Etudiant> etudiants_;
    std::vector<UE> ues_;
    std::vector<Matiere> matieres_;
    std::vector<Salle> salles_;
    std::vector<Note> notes_;
    EmploiDuTemps edt_;

    template <class T, class P>
    static const T* chercher(const std::vector<T>& v, P pred) {
        auto it = std::find_if(v.begin(), v.end(), pred);
        return it == v.end() ? nullptr : &*it;
    }

    // ----- Persistance CSV générique (chaque classe fournit toCsv / fromCsv)
    template <class T>
    static void ecrire(const std::string& chemin, const std::vector<T>& v) {
        std::ofstream f(chemin);
        if (!f) throw std::runtime_error("Impossible d'écrire " + chemin);
        for (const auto& x : v) f << x.toCsv() << "\n";
    }
    template <class T>
    static std::vector<T> lire(const std::string& chemin) {
        std::vector<T> r;
        std::ifstream f(chemin);
        if (!f) return r;  // premier lancement : fichier absent
        std::string l;
        while (std::getline(f, l)) {
            if (!l.empty() && l.back() == '\r') l.pop_back();
            if (!l.empty()) r.push_back(T::fromCsv(l));
        }
        return r;
    }

public:
    explicit Departement(std::string nom = "Département Informatique") : nom_(std::move(nom)) {}

    const std::string& nom() const { return nom_; }

    // ----- Consultation
    const std::vector<Filiere>& filieres() const { return filieres_; }
    const std::vector<Niveau>& niveaux() const { return niveaux_; }
    const std::vector<Enseignant>& enseignants() const { return enseignants_; }
    const std::vector<Etudiant>& etudiants() const { return etudiants_; }
    const std::vector<UE>& ues() const { return ues_; }
    const std::vector<Matiere>& matieres() const { return matieres_; }
    const std::vector<Salle>& salles() const { return salles_; }

    const Filiere* filiere(const std::string& c) const { return chercher(filieres_, [&](const Filiere& x) { return x.code() == c; }); }
    const Niveau* niveau(const std::string& c) const { return chercher(niveaux_, [&](const Niveau& x) { return x.code() == c; }); }
    const Enseignant* enseignant(const std::string& i) const { return chercher(enseignants_, [&](const Enseignant& x) { return x.id() == i; }); }
    const Etudiant* etudiant(const std::string& i) const { return chercher(etudiants_, [&](const Etudiant& x) { return x.id() == i; }); }
    const UE* ue(const std::string& c) const { return chercher(ues_, [&](const UE& x) { return x.code() == c; }); }
    const Matiere* matiere(const std::string& c) const { return chercher(matieres_, [&](const Matiere& x) { return x.code() == c; }); }
    const Salle* salle(const std::string& c) const { return chercher(salles_, [&](const Salle& x) { return x.code() == c; }); }

    // ----- Gestion (validations métier)
    void ajouterFiliere(const Filiere& f) {
        if (filiere(f.code())) throw std::runtime_error("Filière déjà existante");
        filieres_.push_back(f);
    }
    void ajouterNiveau(const Niveau& n) {
        if (niveau(n.code())) throw std::runtime_error("Niveau déjà existant");
        niveaux_.push_back(n);
    }
    void ajouterEnseignant(const Enseignant& e) {
        if (enseignant(e.id())) throw std::runtime_error("Enseignant déjà existant");
        enseignants_.push_back(e);
    }
    void ajouterEtudiant(const Etudiant& e) {
        if (etudiant(e.id())) throw std::runtime_error("Étudiant déjà existant");
        if (!filiere(e.filiere())) throw std::runtime_error("Filière inconnue");
        if (!niveau(e.niveau())) throw std::runtime_error("Niveau inconnu");
        etudiants_.push_back(e);
    }
    void ajouterUE(const UE& u) {
        if (ue(u.code())) throw std::runtime_error("UE déjà existante");
        if (u.credits() <= 0) throw std::runtime_error("Crédits invalides");
        if (!filiere(u.filiere()) || !niveau(u.niveau())) throw std::runtime_error("Filière ou niveau inconnu");
        ues_.push_back(u);
    }
    void ajouterMatiere(const Matiere& m) {
        if (matiere(m.code())) throw std::runtime_error("Matière déjà existante");
        if (m.coefficient() <= 0) throw std::runtime_error("Coefficient invalide");
        if (!ue(m.codeUE())) throw std::runtime_error("UE inconnue");
        if (!enseignant(m.idEnseignant())) throw std::runtime_error("Enseignant inconnu");
        matieres_.push_back(m);
    }
    void ajouterSalle(const Salle& s) {
        if (salle(s.code())) throw std::runtime_error("Salle déjà existante");
        if (s.capacite() <= 0) throw std::runtime_error("Capacité invalide");
        salles_.push_back(s);
    }
    void planifierSeance(const Seance& s) {
        if (!matiere(s.codeMatiere())) throw std::runtime_error("Matière inconnue");
        if (!enseignant(s.idEnseignant())) throw std::runtime_error("Enseignant inconnu");
        if (!salle(s.codeSalle())) throw std::runtime_error("Salle inconnue");
        if (!filiere(s.filiere()) || !niveau(s.niveau())) throw std::runtime_error("Filière ou niveau inconnu");
        if (s.jour() < 1 || s.jour() > 6) throw std::runtime_error("Jour invalide (1 à 6)");
        if (s.debut() < 0 || s.fin() > 24 || s.debut() >= s.fin()) throw std::runtime_error("Horaires invalides");
        edt_.ajouter(s);  // lève une exception en cas de conflit
    }
    void saisirNote(const std::string& idEtu, const std::string& codeMat, double valeur) {
        if (!etudiant(idEtu)) throw std::runtime_error("Étudiant inconnu");
        if (!matiere(codeMat)) throw std::runtime_error("Matière inconnue");
        if (valeur < 0 || valeur > 20) throw std::runtime_error("La note doit être comprise entre 0 et 20");
        for (auto& n : notes_)
            if (n.idEtudiant() == idEtu && n.codeMatiere() == codeMat) { n.setValeur(valeur); return; }
        notes_.emplace_back(idEtu, codeMat, valeur);
    }

    // ----- Calculs académiques
    // Moyenne de l'UE = Σ(note × coef) / Σ coef ; retourne -1 si une note manque
    double moyenneUE(const std::string& idEtu, const UE& u) const {
        double somme = 0, coefs = 0;
        for (const auto& m : matieres_) {
            if (m.codeUE() != u.code()) continue;
            const Note* n = chercher(notes_, [&](const Note& x) { return x.idEtudiant() == idEtu && x.codeMatiere() == m.code(); });
            if (!n) return -1;
            somme += n->valeur() * m.coefficient();
            coefs += m.coefficient();
        }
        return coefs > 0 ? somme / coefs : -1;
    }
    std::vector<const UE*> uesDe(const Etudiant& e) const {
        std::vector<const UE*> r;
        for (const auto& u : ues_) if (u.filiere() == e.filiere() && u.niveau() == e.niveau()) r.push_back(&u);
        return r;
    }
    int creditsTotal(const std::string& idEtu) const {
        const Etudiant* e = etudiant(idEtu);
        int t = 0;
        if (e) for (auto* u : uesDe(*e)) t += u->credits();
        return t;
    }
    int creditsValides(const std::string& idEtu) const {
        const Etudiant* e = etudiant(idEtu);
        int t = 0;
        if (e) for (auto* u : uesDe(*e)) if (moyenneUE(idEtu, *u) >= 10.0) t += u->credits();
        return t;
    }
    bool notesCompletes(const std::string& idEtu) const {
        const Etudiant* e = etudiant(idEtu);
        if (!e) return false;
        for (auto* u : uesDe(*e)) if (moyenneUE(idEtu, *u) < 0) return false;
        return true;
    }
    // Moyenne générale pondérée par les crédits ; -1 si aucune UE complète
    double moyenneGenerale(const std::string& idEtu) const {
        const Etudiant* e = etudiant(idEtu);
        if (!e) return -1;
        double somme = 0; int credits = 0;
        for (auto* u : uesDe(*e)) {
            double m = moyenneUE(idEtu, *u);
            if (m < 0) continue;
            somme += m * u->credits();
            credits += u->credits();
        }
        return credits > 0 ? somme / credits : -1;
    }
    std::string decision(const std::string& idEtu) const {
        if (creditsTotal(idEtu) == 0) return "AUCUNE UE";
        if (!notesCompletes(idEtu)) return "INCOMPLET (notes manquantes)";
        if (creditsValides(idEtu) == creditsTotal(idEtu)) return "ADMIS";
        if (moyenneGenerale(idEtu) >= 10.0) return "ADMIS PAR COMPENSATION";
        return "AJOURNE";
    }

    // ----- Relevé de notes et délibération
    std::string genererReleve(const std::string& idEtu) const {
        const Etudiant* e = etudiant(idEtu);
        if (!e) throw std::runtime_error("Étudiant inconnu");
        std::ostringstream o;
        o << std::fixed << std::setprecision(2);
        o << "=========== RELEVE DE NOTES - " << nom_ << " ===========\n";
        o << "Etudiant : " << e->nomComplet() << " (" << e->id() << ")\n";
        o << "Filiere  : " << e->filiere() << "   Niveau : " << e->niveau() << "\n\n";
        for (auto* u : uesDe(*e)) {
            double mu = moyenneUE(idEtu, *u);
            o << u->code() << " - " << u->intitule() << " (" << u->credits() << " credits)\n";
            for (const auto& m : matieres_) {
                if (m.codeUE() != u->code()) continue;
                const Note* n = chercher(notes_, [&](const Note& x) { return x.idEtudiant() == idEtu && x.codeMatiere() == m.code(); });
                o << "   " << m.code() << " " << m.intitule() << " (coef " << m.coefficient() << ") : ";
                if (n) o << n->valeur(); else o << "--";
                o << "\n";
            }
            if (mu < 0) o << "   Moyenne UE : -- (incomplete)\n";
            else o << "   Moyenne UE : " << mu << (mu >= 10 ? "  -> VALIDEE" : "  -> NON VALIDEE") << "\n";
        }
        double mg = moyenneGenerale(idEtu);
        o << "\nMoyenne generale : ";
        if (mg < 0) o << "--"; else o << mg;
        o << "\nCredits valides  : " << creditsValides(idEtu) << " / " << creditsTotal(idEtu);
        o << "\nDecision         : " << decision(idEtu) << "\n";
        return o.str();
    }
    std::string deliberer(const std::string& codeFiliere, const std::string& codeNiveau) const {
        std::ostringstream o;
        o << std::fixed << std::setprecision(2);
        o << "--- Deliberation " << codeFiliere << "/" << codeNiveau << " ---\n";
        for (const auto& e : etudiants_) {
            if (e.filiere() != codeFiliere || e.niveau() != codeNiveau) continue;
            double mg = moyenneGenerale(e.id());
            o << e.id() << "  " << e.nomComplet() << " | moyenne : ";
            if (mg < 0) o << "--"; else o << mg;
            o << " | credits : " << creditsValides(e.id()) << "/" << creditsTotal(e.id())
              << " | " << decision(e.id()) << "\n";
        }
        return o.str();
    }

    // ----- Emploi du temps
    std::string afficherEDT(const std::string& filiere, const std::string& niveau) const {
        static const char* jours[] = {"", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi"};
        std::ostringstream o;
        o << "--- Emploi du temps " << filiere << "/" << niveau << " ---\n";
        for (const auto& s : edt_.pour(filiere, niveau)) {
            const Matiere* m = matiere(s.codeMatiere());
            o << jours[s.jour()] << " " << s.debut() << "h-" << s.fin() << "h : "
              << (m ? m->intitule() : s.codeMatiere()) << " | salle " << s.codeSalle()
              << " | ens. " << s.idEnseignant() << "\n";
        }
        return o.str();
    }

    // ----- Persistance
    void sauvegarder(const std::string& dossier) const {
        creerDossier(dossier);
        ecrire(dossier + "/filieres.csv", filieres_);
        ecrire(dossier + "/niveaux.csv", niveaux_);
        ecrire(dossier + "/enseignants.csv", enseignants_);
        ecrire(dossier + "/etudiants.csv", etudiants_);
        ecrire(dossier + "/ues.csv", ues_);
        ecrire(dossier + "/matieres.csv", matieres_);
        ecrire(dossier + "/salles.csv", salles_);
        ecrire(dossier + "/notes.csv", notes_);
        ecrire(dossier + "/seances.csv", edt_.toutes());
    }
    void charger(const std::string& dossier) {
        filieres_ = lire<Filiere>(dossier + "/filieres.csv");
        niveaux_ = lire<Niveau>(dossier + "/niveaux.csv");
        enseignants_ = lire<Enseignant>(dossier + "/enseignants.csv");
        etudiants_ = lire<Etudiant>(dossier + "/etudiants.csv");
        ues_ = lire<UE>(dossier + "/ues.csv");
        matieres_ = lire<Matiere>(dossier + "/matieres.csv");
        salles_ = lire<Salle>(dossier + "/salles.csv");
        notes_ = lire<Note>(dossier + "/notes.csv");
        edt_ = EmploiDuTemps();
        for (const auto& s : lire<Seance>(dossier + "/seances.csv")) edt_.ajouter(s);
    }
};

// TP 4 - Système intégré de gestion d'un département universitaire
// Compilation : g++ -std=c++17 -Wall -Wextra -o gestion_departement main.cpp

static const std::string DOSSIER = "data";

static std::string lireTexte(const std::string& question) {
    std::cout << question << " : ";
    std::string s;
    if (!std::getline(std::cin, s)) throw std::runtime_error("Fin de saisie");
    if (s.empty()) throw std::runtime_error("Valeur obligatoire");
    if (s.find(';') != std::string::npos) throw std::runtime_error("Le caractère ';' est interdit");
    return s;
}
static int lireEntier(const std::string& q) {
    try { return std::stoi(lireTexte(q)); } catch (const std::invalid_argument&) { throw std::runtime_error("Nombre invalide"); }
}
static double lireReel(const std::string& q) {
    try { return std::stod(lireTexte(q)); } catch (const std::invalid_argument&) { throw std::runtime_error("Nombre invalide"); }
}

static void donneesDemo(Departement& d) {
    d.ajouterFiliere(Filiere("INFO", "Informatique"));
    d.ajouterNiveau(Niveau("L2", "Licence 2"));
    d.ajouterEnseignant(Enseignant("ENS1", "Kone", "Awa", "Programmation"));
    d.ajouterEnseignant(Enseignant("ENS2", "Diallo", "Moussa", "Mathematiques"));
    d.ajouterSalle(Salle("S101", 60));
    d.ajouterSalle(Salle("S102", 40));
    d.ajouterUE(UE("UE1", "Programmation", 6, "INFO", "L2"));
    d.ajouterUE(UE("UE2", "Mathematiques", 4, "INFO", "L2"));
    d.ajouterMatiere(Matiere("POO", "Programmation orientee objet", 2, "UE1", "ENS1"));
    d.ajouterMatiere(Matiere("ALGO", "Algorithmique", 1, "UE1", "ENS1"));
    d.ajouterMatiere(Matiere("ANAL", "Analyse", 1, "UE2", "ENS2"));
    d.ajouterEtudiant(Etudiant("E001", "Traore", "Fatou", "INFO", "L2"));
    d.ajouterEtudiant(Etudiant("E002", "Sow", "Ibrahim", "INFO", "L2"));
    d.ajouterEtudiant(Etudiant("E003", "Ba", "Mariam", "INFO", "L2"));
    d.planifierSeance(Seance("POO", "ENS1", "S101", "INFO", "L2", 1, 8, 10));
    d.planifierSeance(Seance("ANAL", "ENS2", "S102", "INFO", "L2", 2, 10, 12));
    const char* ids[] = {"E001", "E002", "E003"};
    const double notes[3][3] = {{14, 12, 11}, {8, 9, 15}, {6, 7, 5}};  // POO, ALGO, ANAL
    const char* mats[] = {"POO", "ALGO", "ANAL"};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) d.saisirNote(ids[i], mats[j], notes[i][j]);
}

static void menu() {
    std::cout << "\n===== Gestion du departement =====\n"
                 " 1. Ajouter un etudiant          2. Lister les etudiants\n"
                 " 3. Ajouter un enseignant        4. Lister les enseignants\n"
                 " 5. Ajouter une filiere          6. Ajouter un niveau\n"
                 " 7. Lister filieres et niveaux   8. Ajouter une UE\n"
                 " 9. Ajouter une matiere         10. Lister UE et matieres\n"
                 "11. Ajouter une salle           12. Planifier une seance\n"
                 "13. Afficher un emploi du temps 14. Saisir une note\n"
                 "15. Releve de notes             16. Deliberation\n"
                 "17. Charger des donnees de demonstration\n"
                 " 0. Quitter\n";
}

int main() {
    Departement dep;
    try { dep.charger(DOSSIER); } catch (const std::exception& e) { std::cerr << "Chargement impossible : " << e.what() << "\n"; }

    while (true) {
        menu();
        int choix = -1;
        try {
            choix = lireEntier("Choix");
            switch (choix) {
            case 0: dep.sauvegarder(DOSSIER); std::cout << "Donnees sauvegardees. Au revoir.\n"; return 0;
            case 1: {
                std::string id = lireTexte("Matricule"), nom = lireTexte("Nom"), pre = lireTexte("Prenom");
                std::string f = lireTexte("Code filiere"), n = lireTexte("Code niveau");
                dep.ajouterEtudiant(Etudiant(id, nom, pre, f, n));
                break;
            }
            case 2:
                for (const auto& e : dep.etudiants())
                    std::cout << e.id() << " | " << e.nomComplet() << " | " << e.filiere() << "/" << e.niveau() << "\n";
                break;
            case 3: {
                std::string id = lireTexte("Identifiant"), nom = lireTexte("Nom"), pre = lireTexte("Prenom"), sp = lireTexte("Specialite");
                dep.ajouterEnseignant(Enseignant(id, nom, pre, sp));
                break;
            }
            case 4:
                for (const auto& e : dep.enseignants())
                    std::cout << e.id() << " | " << e.nomComplet() << " | " << e.specialite() << "\n";
                break;
            case 5: { std::string c = lireTexte("Code"), n = lireTexte("Nom"); dep.ajouterFiliere(Filiere(c, n)); break; }
            case 6: { std::string c = lireTexte("Code"), l = lireTexte("Libelle"); dep.ajouterNiveau(Niveau(c, l)); break; }
            case 7:
                for (const auto& f : dep.filieres()) std::cout << "Filiere " << f.code() << " : " << f.nom() << "\n";
                for (const auto& n : dep.niveaux()) std::cout << "Niveau  " << n.code() << " : " << n.libelle() << "\n";
                break;
            case 8: {
                std::string c = lireTexte("Code UE"), i = lireTexte("Intitule");
                int cr = lireEntier("Credits");
                std::string f = lireTexte("Code filiere"), n = lireTexte("Code niveau");
                dep.ajouterUE(UE(c, i, cr, f, n));
                break;
            }
            case 9: {
                std::string c = lireTexte("Code matiere"), i = lireTexte("Intitule");
                double coef = lireReel("Coefficient");
                std::string u = lireTexte("Code UE"), e = lireTexte("Id enseignant");
                dep.ajouterMatiere(Matiere(c, i, coef, u, e));
                break;
            }
            case 10:
                for (const auto& u : dep.ues()) {
                    std::cout << u.code() << " - " << u.intitule() << " (" << u.credits() << " cr, " << u.filiere() << "/" << u.niveau() << ")\n";
                    for (const auto& m : dep.matieres())
                        if (m.codeUE() == u.code())
                            std::cout << "    " << m.code() << " " << m.intitule() << " coef " << m.coefficient() << " ens. " << m.idEnseignant() << "\n";
                }
                break;
            case 11: { std::string c = lireTexte("Code salle"); int cap = lireEntier("Capacite"); dep.ajouterSalle(Salle(c, cap)); break; }
            case 12: {
                std::string cm = lireTexte("Code matiere");
                const Matiere* m = dep.matiere(cm);
                if (!m) throw std::runtime_error("Matiere inconnue");
                std::string salle = lireTexte("Code salle"), f = lireTexte("Code filiere"), n = lireTexte("Code niveau");
                int jour = lireEntier("Jour (1=lundi ... 6=samedi)"), deb = lireEntier("Heure de debut"), fin = lireEntier("Heure de fin");
                dep.planifierSeance(Seance(cm, m->idEnseignant(), salle, f, n, jour, deb, fin));
                break;
            }
            case 13: {
                std::string f = lireTexte("Code filiere"), n = lireTexte("Code niveau");
                std::cout << dep.afficherEDT(f, n);
                break;
            }
            case 14: {
                std::string e = lireTexte("Matricule etudiant"), m = lireTexte("Code matiere");
                double v = lireReel("Note /20");
                dep.saisirNote(e, m, v);
                break;
            }
            case 15: {
                std::string id = lireTexte("Matricule etudiant");
                std::string releve = dep.genererReleve(id);
                std::cout << releve;
                creerDossier("releves");
                std::ofstream fichier(("releves/releve_" + id + ".txt").c_str());
                fichier << releve;
                std::cout << "(releve enregistre dans releves/releve_" << id << ".txt)\n";
                break;
            }
            case 16: {
                std::string f = lireTexte("Code filiere"), n = lireTexte("Code niveau");
                std::cout << dep.deliberer(f, n);
                break;
            }
            case 17: donneesDemo(dep); std::cout << "Donnees de demonstration chargees.\n"; break;
            default: std::cout << "Choix invalide.\n";
            }
            if (choix != 0) dep.sauvegarder(DOSSIER);
        } catch (const std::exception& e) {
            if (std::cin.eof()) { std::cout << "\nFin de saisie.\n"; return 0; }
            std::cout << "Erreur : " << e.what() << "\n";
        }
    }
}
