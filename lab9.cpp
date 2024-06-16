#include <iostream>
#include <random>
#include <memory>
#include <algorithm>
#include <map>
using namespace std;

random_device rd;

class Entity {
private:
    string name;
    int health;
    int damage;
    uniform_int_distribution<int> valueRange = uniform_int_distribution(-20, 20);
public:
    bool alive = true;
    Entity(const string& initName, int baseHealth, int baseDamage) {
        name = initName;
        health = baseHealth;
        damage = baseDamage;
    }
    string getName() {
        return name;
    }
    int getDamage() {
        return damage;
    }
    int getHealth() {
        return health;
    }
    uniform_int_distribution<int> getRange() {
        return valueRange;
    }
    void takeDamage(int dealtDamage) {
        health -= dealtDamage;
        if (health <= 0)
            alive = false;
    }
    void heal(int healValue) {
        health += healValue;
    }
    virtual void attack(const shared_ptr<Entity>& other) {
        int damagedFor = valueRange(rd) + damage;
        other->takeDamage(damagedFor);
        cout << name << " ATTACKED " << other->name << " FOR " << damagedFor << endl;
        if (!other->alive) {
            throw other->name;
        }
    }
    void show() {
        cout << name << " " << health << " " << damage << endl;
    }
    virtual string getType() = 0;
    virtual void healTo(const shared_ptr<Entity>& other) {}
};


class Enemy : public Entity {
public:
    explicit Enemy(const string& name) : Entity(name, 70, 25) {}
    string getType() override {
        return "Enemy";
    }
};

class Mage : public Entity {
public:
    explicit Mage(const string& name) : Entity(name, 60, 60) {}
    string getType() override {
        return "mage";
    }
};

class Priest : public Entity {
public:
    explicit Priest(const string& name) : Entity(name, 60, 40) {}
    void healTo(const shared_ptr<Entity>& other) override {
        int healDealt = getRange()(rd) + getDamage();
        other->heal(healDealt);
        cout << getName() << " HEALED " << other->getName() << " FOR " << getDamage() << endl;
    }
    string getType() override {
        return "priest";
    }
};

class Warrior : public Entity {
public:
    explicit Warrior(const string& name) : Entity(name, 100, 50) {}
    string getType() override {
        return "warrior";
    }
};

class Rogue : public Entity {
public:
    explicit Rogue(const string& name) : Entity(name, 50, 60) {}
    string getType() override {
        return "rogue";
    }
};

class Game {
private:
    uniform_int_distribution<int> charRange;
    uniform_int_distribution<int> enemyRange;
public:
    map<string, shared_ptr<Entity>> characters = {
            {"FRIEREN", make_shared<Mage>(Mage("FRIEREN"))},
            {"EISEN", make_shared<Warrior>(Warrior("EISEN"))},
            {"HEITER", make_shared<Priest>(Priest("HEITER"))},
            {"HIMMEL", make_shared<Rogue>(Rogue("HIMMEL"))},
    };

    map<string, shared_ptr<Enemy>> enemies = {
            {"GOBLIN1", make_shared<Enemy>(Enemy("GOBLIN1"))},
            {"GOBLIN2", make_shared<Enemy>(Enemy("GOBLIN2"))},
    };
    void showEntites() {
        cout << "CHARACTERS:" << endl;
        for (const auto& charaPair : characters)
                cout << charaPair.second->getName() << " TYPE:" << charaPair.second->getType() << " HEALTH:"
                     << charaPair.second->getHealth() << " DAMAGE:" << charaPair.second->getDamage() << endl;
        cout << "ENEMIES:" << endl;
        for (const auto& enemyPair : enemies)
                cout << enemyPair.second->getName() << " HEALTH:" << enemyPair.second->getHealth()
                     << " DAMAGE:" << enemyPair.second->getDamage() << endl;
    }
    shared_ptr<Entity> getCharacter(const string& name) {
        return characters.at(name);
    }
    shared_ptr<Enemy> getEnemy(const string& name) {
        return enemies.at(name);
    }
    void enemyAttack() {
        if (enemies.empty())
            return;
        enemyRange = uniform_int_distribution<int>(0, enemies.size() - 1);
        charRange = uniform_int_distribution<int>(0, characters.size() - 1);
        int enemyIx = enemyRange(rd);
        int charIx = charRange(rd);
        auto enIt = enemies.begin();
        auto charIt = characters.begin();
        advance(enIt, enemyIx);
        advance(charIt, charIx);
        auto enemy = enIt->second;
        auto chara = charIt->second;
        try {
            enemy->attack(charIt->second);
        } catch (string& charName) {
            cout << enemy->getName() << " KILLED " << chara->getName() << endl;
            characters.erase(charName);
        }
    }
    Game() {
        showEntites();
        cout << "PICK A CHARACTER" << endl
             << "TYPE 'HELP' FOR HELP" << endl;
    }
    bool over() {
        if (enemies.empty()) {
            cout << "VICTORY" << endl;
            return true;
        } else if (characters.empty()) {
            cout << "YOU LOST" << endl;
            return true;
        }
        return false;
    }
    static void showHelp() {
        cout << "AVAILABLE COMMANDS (NON-CASE-SENSITIVE):" << endl
             << "PICK 'characterName' -> pick a character to make your next moves with" << endl
             << "SHOWALL -> shows all alive characters and enemies" << endl
             << "MOVE -> make one move:" << endl
             << "1) ATTACK 'enemyName' -> attack an enemy with your current character" << endl
             << "2) HEAL 'characterName' -> heal any character with a priest" << endl
             << "AFTER MAKING A MOVE, ENEMIES WILL ATTACK A RANDOM CHARACTER" << endl;
    }
};

void toUpper(string& str) {
    for (auto& c : str)
        c = char(toupper(c));
}

class Player {
private:
    Game* currGame;
    shared_ptr<Entity> chara;
    map<string, void(*)()> funcs = {

    };
public:
    void setCharacter(const shared_ptr<Enemy>& otherCharacter) {
        if (dynamic_pointer_cast<Enemy>(otherCharacter)) {
            cout << "YOU CANT PLAY AS AN ENEMY" << endl;
            return;
        }
        chara = otherCharacter;
    }
    Player() {
        currGame = new Game();
    }
    void playerMove() {
        string move;
        cin >> move;
        toUpper(move);
        if (move != "SKIP" && chara == nullptr) {
            cout << "PICK A CHARACTER FIRST" << endl;
        } else if (move == "ATTACK") {
            string otherName;
            cin >> otherName;
            toUpper(otherName);
            if (dynamic_pointer_cast<Priest>(chara)) {
                cout << "YOU CANT ATTACK AS A PRIEST" << endl;
                return;
            }
            try {
                chara->attack(currGame->getEnemy(otherName));
            } catch (string& enemyName) {
                cout << chara->getName() << " KILLED " << otherName << endl;
                currGame->enemies.erase(enemyName);
            } catch (exception&) {
                cout << "THIS ENEMY DOES NOT EXIST" << endl;
            }
        } else if (move == "HEAL" && (chara = dynamic_pointer_cast<Priest>(chara))) {
            string otherName;
            cin >> otherName;
            toUpper(otherName);
            chara->healTo(currGame->getCharacter(otherName));
        }
        currGame->enemyAttack();
    }
    void pick() {
        string charName;
        cin >> charName;
        toUpper(charName);
        try {
            chara = currGame->getCharacter(charName);
        } catch(exception&) {
            cout << "THIS CHARACTER DOES NOT EXIST" << endl;
        }
        cout << "Picked " << chara->getType() << " " << chara->getName() << endl;
    }
    void showAll() {
        currGame->showEntites();
    }

    Game* getGame() {
        return currGame;
    }
};

int main() {
    auto player1 = new Player();
    string command;
    while (!player1->getGame()->over()){
        cin >> command;
        toUpper(command);
        if (command == "MOVE") {
            player1->playerMove();
        } else if (command == "SHOWALL") {
            player1->showAll();
        } else if (command == "PICK") {
            player1->pick();
        } else if (command == "HELP") {
            Game::showHelp();
        }
    }
}