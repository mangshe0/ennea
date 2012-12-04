#include <string>
#include <algorithm>
#include <functional>
#include <map>
#include <ctime>

using namespace std;




struct Stat
{
	std::string name;
	int current, max;
	int exp, exp_required;
	Stat(std::string name_) : name(name_), current(1), max(1), exp(0), exp_required(80) {}
	Stat(std::string name_, int c) : name(name_), current(c), max(c), exp(0), exp_required(80) {}
	Stat(std::string name_, int c, int m) : name(name_), current(c), max(m), exp(0), exp_required(80) {}
	void increase(int amount) {
		if(amount<0) return;
		current += amount;
		if(current>max+5) current = max+5;
	}
	void decrease(int amount) {
		if(amount<0) return;
		current -= amount;
		if(current<0) current = 0;
	}
	void recover() {
		if(current>max) current--;
		if(current<max) current++;
	}
};

/*template <class T>
class Comparable
{
public:
    Comparable(T v) : value(v) {}
    T value;
    bool operator==(const Comparable& c) const {
        return value == c.value;
    }

    bool operator<(const Comparable& c) const {
        return value < c.value;
    }
};*/




class Object;

struct Ability
{
    Ability() : selfCastAllowed(false), friendlyCastAllowed(false) {}
    virtual ~Ability() { std::cout << "Ability destroyed" << std::endl; }
	virtual void onCast(Object* caster, Object* victim) {}
	bool selfCastAllowed, friendlyCastAllowed;
};

class Action
{
public:
	virtual void onPerform(Object* performer) = 0;
};

class WalkToAction : public Action
{
public:
	WalkToAction(int x_, int y_) : x(x_), y(y_) {}
	virtual ~WalkToAction() {}
	
	virtual void onPerform(Object* performer) {
	}
protected:
	int x,y;
};

class Buff
{
public:
    Buff()
    : ds(0), stacks(1), visible(true), permanent(true), maxStacks(1)
    {
    }
    virtual ~Buff() {
        std::cout << "Buff destroyed" << std::endl;
    }
	bool permanent;
	bool visible;
	int maxStacks;
	int timeLeft; // *0.1 seconds
	int stacks;
	int effectCooldown;
	void end() { timeLeft = 0; }
	virtual int getID() { return 0; }
	virtual void onEffectStart(Object* carry) {}
	void onEffect(Object* carry, int delta_ds) {
		ds += delta_ds;
		while(ds >= effectCooldown) {
			ds -= effectCooldown;
			onEffect(carry);
		}
	}
	virtual void onEffect(Object* carry) {}
	virtual void onEffectEnd(Object* carry) { std::cout << "buff ends" << std::endl; }
	virtual void onFullStacked(Object* carry) {}
private:
	int ds;
};

class Object
{
public:
	Object() : lastUpdate(clock()), actionEnd(clock()) {}
	virtual ~Object() {
	    std::cout << name << " deleted" << std::endl;
		for(Buffs::iterator i = buffs.begin() ; i != buffs.end() ; ++i) {
		    delete(i->second);
		}
	}
	std::string name;
	Stat hp;
	Action currentAction;
	clock_t actionEnd;
	time_t lastUpdate;

	void applyBuff(Buff* buff_) {
	    Buff* buff = new Buff(*buff_);
		Buffs::iterator i = buffs.find(buff->getID());
		if(i != buffs.end()) {
			// Found, refresh the old buff and add one stack if necessary
			buff->stacks = i->second->stacks + 1;
			delete(i->second);
			buffs.erase(i);
		}
		buffs[buff->getID()] = buff;
		buff->onEffectStart(this);
		if(buff->stacks >= buff->maxStacks) {
			buff->stacks = buff->maxStacks;
			buff->onFullStacked(this);
		}
	}

	void update() {
		clock_t current = clock();
		int diff = (current-lastUpdate)*10 / CLOCKS_PER_SEC;
		if(diff) {
            for(Buffs::iterator i = buffs.begin(); i != buffs.end(); ++i) {
                Buff* buff = i->second;
                buff->timeLeft -= diff;
                std::cout << buff->timeLeft << std::endl;
                if(buff->timeLeft > 0) {
                    buff->onEffect(this, diff);
                } else {
                    buff->onEffectEnd(this);
                   // delete(i->second);
                    buffs.erase(i);
                }
            }
            lastUpdate = current;
		}

	}

	virtual void onDamage(int amount) {
		hp.decrease(amount);
		if(!isAlive()) onDeath();
	}

	// what happens when hp drops below 0
	virtual void onDeath() {}

	bool isAlive() { return hp.current>0; }

	void recover() {
		hp.recover();
		for(Resistances::iterator i = resistance.begin() ; i != resistance.end() ; i++)
            i->second.recover();
	}
	void performAction() {
	    if(clock()>actionEnd) {
            switch(currentAction.id) {
                case 2:
                    currentAction.ability->onCast(this,currentAction.victim);
                    actionEnd = clock() + 1.5 * CLOCKS_PER_SEC;
                    currentAction.id = -1;
                    break;
                case 3:
                    currentAction.victim->damage(DamageType::Physical(),strength.current);
                    actionEnd = clock() + 1.2 * CLOCKS_PER_SEC;
                    break;
                default:
                    break;
            }
		}
	}

  virtual void onGainExp(std::string skillname, int amount) {}
	virtual void onLevelup(std::string skillname, int lvl) {}

	void addStat(Stat s) {
		stats[s.name] = s;
	}

	Stat& getStat(std::string name) {
		return stats[name];
	}

//private:
  typedef map<std::string,Stat> Stats;
	typedef map<int,Buff*> Buffs;
	Stats stats;
	Buffs buffs;
};

