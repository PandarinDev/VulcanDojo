#include <iostream>
#include <chrono>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <queue>
#include <algorithm>
#include <utility>
#include <numeric>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <cmath>

using namespace std;

using uchar = unsigned int;

namespace Utils
{
    template<typename Out>
    static void split(const std::string &s, char delim, Out result) 
    {
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            *(result++) = item;
        }
    }

    static std::vector<std::string> split(const std::string &s, char delim) 
    {
        std::vector<std::string> elems;
        split(s, delim, std::back_inserter(elems));
        return elems;
    }

    template<class T>
    bool vecEqual(const vector<T>& v1, const vector<T>& v2)
    {
        if(v1.size() != v2.size())
            return false;
        for(size_t i = 0; i < v1.size(); i++)
        {
            if(v1[i] != v2[i])
            {
                return false;
            }
        }
        return true;
    }

    template<class T>
    bool arrayEqual(const T& v1, const T& v2)
    {
        for(auto i = 0; i < v1.size(); i++)
        {
            if(v1[i] != v2[i])
            {
                return false;
            }
        }
        return true;
    }

    string toString(const string& c)
    {
        return c;
    }

    auto join = [](const auto& collection, auto separator) -> auto
    {
        stringstream result;
        for(size_t i=0; i < collection.size(); ++i)
        {
            result << toString(collection[i]);
            if(i+1 != collection.size())
                result << separator;
        }
        return result.str();
    };

    auto joinSize = [](const auto& collection, auto& size, auto separator) -> auto
    {
        stringstream result;
        for(size_t i=0; i < size; ++i)
        {
            result << toString(collection[i]);
            if(i+1 != size)
                result << separator;
        }
        return result.str();
    };
    
    template<class T>
    string toString(const vector<T>& cards)
    {
        string r = Utils::join(cards, '\n');
        return r;
    }
    
    template<class T>
    string toString(const T& cards, char size)
    {
        string r = Utils::joinSize(cards, size, '\n');
        return r;
    }
};

namespace CCG
{

enum class BoardLocation : char
{
    EnemySide = -1,
    InHand = 0,
    PlayerSide = 1,
    PlayerSidePassive = 2
};

ostream& operator<<(ostream& os, const BoardLocation& t)
{
	return os << static_cast<int>(t);
};

enum class CardType : char
{
    Creature = 0,
    GreenItem = 1,
    RedItem = 2,
    BlueItem = 3
};

ostream& operator<<(ostream& os, const CardType& t)
{
	return os << static_cast<int>(t);
};

enum class CardAbility : unsigned char
{
    Nothing = 0x00,
    Breakthrough = 0x01,
    Charge = 0x02,
    Drain = 0x04,
    Guard = 0x08,
    Lethal = 0x10,
    Ward = 0x20,
    All = 0x3F,
};

CardAbility operator&(const CardAbility& a, const CardAbility& b)
{
	const auto aa = static_cast<underlying_type_t<CardAbility>>(a);
	const auto ab = static_cast<underlying_type_t<CardAbility>>(b);
	return static_cast<CardAbility>(aa & ab);
}

CardAbility operator|(const CardAbility& a, const CardAbility& b)
{
	const auto aa = static_cast<underlying_type_t<CardAbility>>(a);
	const auto ab = static_cast<underlying_type_t<CardAbility>>(b);
	return static_cast<CardAbility>(aa | ab);
}

void operator|=(CardAbility& a, const CardAbility& b)
{
	const auto aa = static_cast<underlying_type_t<CardAbility>>(a);
	const auto ab = static_cast<underlying_type_t<CardAbility>>(b);
	a = static_cast<CardAbility>(aa | ab);
}

CardAbility operator~(const CardAbility& a)
{
	const auto aa = static_cast<underlying_type_t<CardAbility>>(a);
	return static_cast<CardAbility>(~aa);
}

struct Card
{
    char CardNumber = 0;
    char InstanceId = 0;
    BoardLocation Location = BoardLocation::InHand;
    CardType Type = CardType::Creature;
	char Cost = 0;
	char AttackValue = 0;
	char DefenseValue = 0;
    CardAbility Abilities = CardAbility::Nothing;
	char MyHealthChange = 0;
	char EnemyHealthChange = 0;
	char CardDraw = 0;

    uchar StatusFlags = 0; // Summoned + did attack
    
    Card() = default;
    Card(const Card&) = default;
    Card(Card&&) = default;
    Card& operator=(const Card&) = default;

    static constexpr uchar DidAttackFlag = 0x01;
    static constexpr uchar SummonedFlag = 0x02;

    bool GetDidAttack() const { return bool(StatusFlags & DidAttackFlag);}
    bool GetJustSummoned() const { return bool(StatusFlags & SummonedFlag);}
    
    void SetDidAttack(bool f)    { StatusFlags |= DidAttackFlag & f;}
    void SetJustSummoned(bool f) { StatusFlags |= SummonedFlag & f << 1;}

    bool HasBreakthrough() const { return HasAbility(CardAbility::Breakthrough); }
    bool HasCharge() const { return HasAbility(CardAbility::Charge); }
    bool HasDrain() const { return HasAbility(CardAbility::Drain); }
    bool HasGuard() const { return HasAbility(CardAbility::Guard); }
    bool HasLethal() const { return HasAbility(CardAbility::Lethal); }
    bool HasWard() const { return HasAbility(CardAbility::Ward); }
    bool HasAbility(CardAbility a) const {
        return (Abilities & a) != CardAbility::Nothing;
    }

    void AddBreakthrough() { AddAbility(CardAbility::Breakthrough); }
    void AddCharge() { AddAbility(CardAbility::Charge);}
    void AddDrain() { AddAbility(CardAbility::Drain);}
    void AddGuard() { AddAbility(CardAbility::Guard);}
    void AddLethal() { AddAbility(CardAbility::Lethal);}
    void AddWard() { AddAbility(CardAbility::Ward);}
    void AddAbility(CardAbility a) {
        Abilities = (Abilities | a);
    }

    void RemoveBreakthrough() { RemoveAbilty(CardAbility::Breakthrough); }
    void RemoveCharge() { RemoveAbilty(CardAbility::Charge);}
    void RemoveDrain() { RemoveAbilty(CardAbility::Drain);}
    void RemoveGuard() { RemoveAbilty(CardAbility::Guard);}
    void RemoveLethal() { RemoveAbilty(CardAbility::Lethal);}
    void RemoveWard() { RemoveAbilty(CardAbility::Ward);}
    void RemoveAbilty(CardAbility a) {
        Abilities = ((Abilities & ~a) & CardAbility::All);
    }

    string ToString() const
    {
        stringstream ss;
		ss << (int)CardNumber << " " << (int)InstanceId << " ";
		ss << Location << " " << Type << " " << (int)Cost << " ";
		ss << (int)AttackValue << " " << (int)DefenseValue << " ";
		ss << AbilitiesToString() << " " << (int)MyHealthChange << " ";
    	ss << (int)EnemyHealthChange << " " << (int)CardDraw;
        return ss.str();
    }

    ostream& operator<<(ostream& os)
    {
        return os << ToString();
    }

    string AbilitiesToString() const
    {
        string abilities = HasBreakthrough() ? "B" : "-";
        abilities += HasCharge() ? "C" : "-";
        abilities += HasDrain() ? "D" : "-";
        abilities += HasGuard() ? "G" : "-";
        abilities += HasLethal() ? "L" : "-";
        abilities += HasWard() ? "W" : "-";
        return abilities;
    }

    bool operator==(const Card& c) const
    {
		return (memcmp(this, &c, sizeof(Card)) == 0);
    }

    bool operator!=(const Card& c) const
    {
        return !(*this == c);
    }

    bool operator!=(const Card& c)
    {
        return !(*this == c);
    }
};

std::ostream& operator<<(std::ostream& os, const Card& c)
{
    return os << c.ToString();
}

string toString(const Card& c)
{
    return c.ToString();
}

enum ActionType
{
    CreatureAttack,
    SummonCreature,
    UseItem
};

struct GameAction
{
    static const int EnemyPlayerId = -1;

    ActionType Type;
    int Id = 0;
    int TargetId = EnemyPlayerId;

    GameAction(ActionType type) : Type(type) {}

    GameAction(ActionType type, int iid) : GameAction(type)
    {
        Id = iid;
    }

    GameAction(ActionType type, int iid, int targetId) : GameAction(type, iid)
    {
        TargetId = targetId;
    }

    string ToString() const
    {
        stringstream ss;
        switch(Type)
        {
            case ActionType::CreatureAttack:
                ss << "ATTACK " << Id << " " << TargetId;
                break;
            case ActionType::SummonCreature:
                ss << "SUMMON " << Id;
                break;
            case ActionType::UseItem:
                ss << "USE " << Id << " " << TargetId;
                break;
            default:
                ss << "PASS";
        }
        return ss.str();
    }

    bool operator==(const GameAction& a) const
    {
        return (Type == a.Type && Id == a.Id && TargetId == a.TargetId);
    }
};

string toString(const GameAction& a)
{
    return a.ToString();
}

struct ActionSequence
{
    vector<GameAction> Actions;

    string ToString() const
    {
        string actions = Utils::join(Actions, ';');
        return (actions == "") ? "PASS" : actions;
    }
    
    ActionSequence() = default;
    ActionSequence(const ActionSequence&) = default;
    ActionSequence& operator=(const ActionSequence&) = default;

    ActionSequence Extended(const GameAction& a) const
    {
        ActionSequence copy(*this);
        copy.Add(a);
        return copy;
    }

    void Add(const GameAction& a)
    {
        Actions.emplace_back(a);
    }
};

namespace GameActionFactory
{
    static GameAction SummonCreature(int iid)
    {
        return GameAction(ActionType::SummonCreature, iid);
    }

    static GameAction CreatureAttack(int iid, int targetId)
    {
        return GameAction(ActionType::CreatureAttack, iid, targetId);
    }

    static GameAction UseItem(int iid, int targetId)
    {
        return GameAction(ActionType::UseItem, iid, targetId);
    }
};

struct Gambler
{
    int Health;
    int Mana;
    int DeckSize;
    int NextRuneThreshold;
    
    Gambler() = default;
    Gambler(const Gambler&) = default;
    Gambler(Gambler&&) = default;
    Gambler& operator=(const Gambler&) = default;

    string ToString() const
    {
        ostringstream ss;
        ss << Health << " " << Mana << " "<< DeckSize <<" "<< NextRuneThreshold;
        return ss.str();
    }

    bool operator==(Gambler c)
    {
        bool result = Health == c.Health && Mana == c.Mana &&
            DeckSize == c.DeckSize && NextRuneThreshold == c.NextRuneThreshold;
        return result;
    }
};

void printError(const char* msg)
{
    cerr << msg << endl;
}
void printError(const string& msg)
{
    cerr << msg << endl;
}

struct GameState
{
    Gambler MyPlayer;
    Gambler EnemyPlayer;
    char EnemyHandCount;
    char CardCount; // Cards in my hand + on the board
    
    array<Card, 8> MyHand;
    array<Card, 6> MyBoard;
    array<Card, 6> EnemyBoard;

    char MyHandCount = 0;
    char MyBoardCount = 0;
    char EnemyBoardCount = 0;
    
    GameState() = default;
    GameState(const GameState&) = default;
    GameState(GameState&&) = default;
	GameState& operator=(GameState& gs) = default;
  //  GameState& operator=(GameState& gs)
  //  {
		//memcpy(this, reinterpret_cast<void *>(&gs), sizeof(GameState));
		//return *this;
  //  }

    string ToString() const
    {
        vector<string> a{
            Utils::toString(MyHand, MyHandCount), 
            Utils::toString(MyBoard, MyBoardCount), 
            Utils::toString(EnemyBoard, EnemyBoardCount)
        };
		vector<string> b;
		copy_if(a.begin(), a.end(), std::back_inserter(b), [](auto a) {return a != ""; });
		string cards = Utils::toString(b);
        stringstream ss;
        ss << MyPlayer.ToString() << endl << EnemyPlayer.ToString() << endl << (int)EnemyHandCount << endl << (int)CardCount << endl << cards;
        return ss.str();
    }

    bool operator==(const GameState& c)
    {
        bool result = 
            MyPlayer == c.MyPlayer &&
            EnemyPlayer == c.EnemyPlayer &&
            EnemyHandCount == c.EnemyHandCount &&
            CardCount == c.CardCount &&
            Utils::arrayEqual(MyHand, c.MyHand) &&
            Utils::arrayEqual(MyBoard, c.MyBoard) &&
            Utils::arrayEqual(EnemyBoard, c.EnemyBoard);
        return result;
    }

    void AddCardToMyHand(const Card& c) { AddCardToArray(MyHand, MyHandCount, c); }
    void AddCardToMyBoard(const Card& c) { AddCardToArray(MyBoard, MyBoardCount, c); }
    void AddCardToEnemyBoard(const Card& c) { AddCardToArray(EnemyBoard, EnemyBoardCount, c); }

    template<class T>
    inline static void AddCardToArray(T& arr, char& counter, const Card& c)
    {
        arr[counter] = c;
        counter++;
    }

    void RemoveCardFromMyHand(const array<Card, 8>::iterator& it) { RemoveCardFromArray(MyHand, MyHandCount, it); }
    void RemoveCardFromMyBoard(const array<Card, 6>::iterator& it) { RemoveCardFromArray(MyBoard, MyBoardCount, it); }
    void RemoveCardFromEnemyBoard(const array<Card, 6>::iterator& it) { RemoveCardFromArray(EnemyBoard, EnemyBoardCount, it); }

	template<class T>
	static void RemoveCardFromArray(T& arr, char& counter, const typename T::iterator& it)
	{
		if (counter <= 0)
			return;
		counter--;
		*it = arr[counter];
		memset(arr.data() + counter, 0, sizeof(Card));
	}

    auto MyHandEnd() const { return MyHand.begin()+MyHandCount; }
    auto MyBoardEnd() const { return MyBoard.begin()+MyBoardCount; }
    auto EnemyBoardEnd() const { return EnemyBoard.begin()+EnemyBoardCount; }
};

class Stopwatch
{
    chrono::time_point<chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();

public:
    Stopwatch() = default;

    void Restart()
    {
        start = std::chrono::high_resolution_clock::now();
    }

    long ElapsedMilliseconds()
    {
        std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now()-start;
        return long(elapsed.count() * 1000.0f);
    }
};


namespace Simulator
{
    void Drain(Gambler& healedPlayer, const Card& attacker, int amount)
    {
        if(attacker.HasDrain())
        {
            healedPlayer.Health += max(0, amount);
        }
        //healedPlayer.Health += ((int)(attacker.Abilities & Card.Ability.Drain) >> 2) * Math.Max(0, amount);
    }    

    void HalfAttack(Card& attacker, Card& defender)
    {
        if(attacker.AttackValue > 0)
        {
            if(defender.HasWard())
                defender.RemoveWard();
            else if(attacker.HasLethal())
            {
                defender.DefenseValue = 0;
            }
            else
                defender.DefenseValue -= attacker.AttackValue;
        }
        attacker.SetDidAttack(true);
    }

    void AttackCreature(Card& attacker, Card& defender)
    {
        HalfAttack(attacker, defender);

        if(!defender.GetDidAttack())
        {
            HalfAttack(defender, attacker);
        }
    }

    void AttackAction(GameState& state, int attackerId, int defenderId)
    {
        auto attIndex = find_if(state.MyBoard.begin(), state.MyBoard.begin()+state.MyBoardCount, [=](const Card& c) { return c.InstanceId == attackerId;});
        auto& attacker = *attIndex;

        if(defenderId != GameAction::EnemyPlayerId)
        {
            auto defIndex = find_if(state.EnemyBoard.begin(), state.EnemyBoard.begin()+state.EnemyBoardCount, [=](const Card& c) { return c.InstanceId == defenderId;});
            auto& defender = *defIndex;
            char attackerHpBefore = attacker.DefenseValue;
            char defenderHpBefore = defender.DefenseValue;
            AttackCreature(attacker, defender);
            Drain(state.MyPlayer, attacker, max(0, defenderHpBefore - max(static_cast<char>(0), defender.DefenseValue)));
            if(attacker.DefenseValue <= 0)
            {
                state.RemoveCardFromMyBoard(attIndex);
                state.CardCount -= 1;
            }
            if(defender.DefenseValue <= 0)
            {
                state.RemoveCardFromEnemyBoard(defIndex);
                state.CardCount -= 1;
                if(attacker.HasBreakthrough())
                {
                    state.EnemyPlayer.Health += defender.DefenseValue;
                }
            }
        }
        else
        {
            state.EnemyPlayer.Health -= attacker.AttackValue;
            Drain(state.MyPlayer, attacker, attacker.AttackValue);
            attacker.SetDidAttack(true);
        }
    }

    void UseItemAction(GameState& state, int itemId, int targetId)
    {
        auto itemIndex = find_if(state.MyHand.begin(), state.MyHand.begin()+state.MyHandCount, [=](const Card& c) { return c.InstanceId == itemId;});
        auto& item = *itemIndex;
        state.MyPlayer.Mana -= item.Cost;
        state.CardCount -= 1;

        state.MyPlayer.Health += item.MyHealthChange;
        state.EnemyPlayer.Health += item.EnemyHealthChange;

        if(item.Type == CardType::GreenItem)
        {
            auto& creature = *find_if(state.MyBoard.begin(), state.MyBoard.begin()+state.MyBoardCount, [=](const Card& c) { return c.InstanceId == targetId;});
            creature.AddAbility(item.Abilities);
            creature.AttackValue += item.AttackValue;
            creature.DefenseValue += item.DefenseValue;

            if(creature.HasCharge())
                creature.SetJustSummoned(false);
        }
        else if(item.Type == CardType::RedItem ||
            (item.Type == CardType::BlueItem && targetId != GameAction::EnemyPlayerId))
        {
            auto crIndex = find_if(state.EnemyBoard.begin(), state.EnemyBoard.begin()+state.EnemyBoardCount, [=](const Card& c) { return c.InstanceId == targetId;});
            auto& creature = *crIndex;
            creature.RemoveAbilty(item.Abilities);
			creature.AttackValue = max(creature.AttackValue + item.AttackValue, 0);
			creature.DefenseValue = max(creature.DefenseValue + item.DefenseValue, 0);
            
            if(creature.DefenseValue <= 0)
            {
                state.RemoveCardFromEnemyBoard(crIndex);
                state.CardCount -= 1;
            }
        }
        else if(item.Type == CardType::BlueItem)
        {
            state.EnemyPlayer.Health += item.DefenseValue;
        }

        // calling erase on iterator changes the reference to point to the next item in the vector
        state.RemoveCardFromMyHand(itemIndex);
    }

    void SummonCreatureAction(GameState& state, int creatureId)
    {
        auto toSummonIndex = find_if(state.MyHand.begin(), state.MyHand.begin()+state.MyHandCount, [=](const Card& c) { return c.InstanceId == creatureId;});
        auto& toSummon = *toSummonIndex;
        state.MyPlayer.Mana -= toSummon.Cost;

        if(toSummon.HasCharge())
        {
            toSummon.Location = BoardLocation::PlayerSide;
            state.AddCardToMyBoard(toSummon);
        }
        else
        {
            toSummon.Location = BoardLocation::PlayerSidePassive;
            toSummon.SetJustSummoned(true);
            state.AddCardToMyBoard(toSummon);
        }

        state.RemoveCardFromMyHand(toSummonIndex);
    }

    GameState SimulateAction(const GameState& gs, const GameAction& a)
    {
        GameState state(gs);
        switch(a.Type)
        {
            case ActionType::CreatureAttack:
                AttackAction(state, a.Id, a.TargetId);
                break;
            case ActionType::SummonCreature:
                SummonCreatureAction(state, a.Id);
                break;
            case ActionType::UseItem:
                UseItemAction(state, a.Id, a.TargetId);
                break;
            default:
                break;
        }
        return move(state);
    }
};

namespace DraftPhase
{
    void CurveAdd(int cost, array<int, 8>& curve)
    {
        int place = clamp(cost, 0, 7);
        curve[place] -= 1;
    }

    double ManaCurveAdjust(const Card& card, array<int, 8>& curve)
    {
        int place = clamp((int)card.Cost, 0, 7);
        auto needInCurve = curve[place];
        return needInCurve;
    }
    
    double GetValue(const GameState& gs, const Card& card, array<int, 8>& curve)
    {
        //TODO: improve red and blue item values
        double value = 0.0;
        const double divisor = max((int)card.Cost, 1);
		const double stats = (double)(fabs(card.AttackValue) + fabs(card.DefenseValue)) / divisor;
        const double hpChange = (double)(card.MyHealthChange - card.EnemyHealthChange) / divisor;
		value += stats + hpChange;
        value += card.CardDraw;
       
        array<double, 6> abilityValues;
        if(card.Type == CardType::Creature)
        {
            const array<double, 6> cAbilityValues = {
                0.5 * stats, // Breakthrough
                1.4 * stats, // Charge
                1.1 * stats, // Drain
                1.5 * stats, // Guard
                1.0 * stats, // Lethal
                1.5 * stats, // Ward
            };
            abilityValues = cAbilityValues;
        }
        else if(card.Type == CardType::GreenItem)
        {
            const array<double, 6> cAbilityValues = {
                1.0 + card.AttackValue, // Breakthrough
                2.0 + card.AttackValue, // Charge
                1.2 + card.AttackValue, // Drain
                2.0 + card.DefenseValue, // Guard
                1.0 + card.DefenseValue, // Lethal
                1.0 + 1.5 * card.AttackValue, // Ward
            };
            abilityValues = cAbilityValues;
        }
        else // red, blue items
        {
            const array<double, 6> cAbilityValues = {
                1.0, // Breakthrough
                0.0, // Charge
                1.2 + fabs(card.AttackValue), // Drain
                2.0 + fabs(card.DefenseValue), // Guard
                1.0, // Lethal
                1.5 + fabs(card.DefenseValue), // Ward
            };
            abilityValues = cAbilityValues;
        }
        
        const int abilities = (int)card.Abilities;
        for(size_t i = 0; i < abilityValues.size(); i++)
        {
            value += abilityValues[i] * ((abilities >> i) & 0x01);
        }

        //balance penalty, nerf card "Decimate"
        if(card.CardNumber == 151)
        {
            value -= 92;
        }
        if(card.Type == CardType::Creature && gs.MyPlayer.DeckSize > 10)
        {
            value += ManaCurveAdjust(card, curve);
        }

        return value;
    }

    /// <summary>
    /// Represent a turn in the draft phase, 
    /// basically selects the card that we should pick
    /// </summary>
    string GetBestCard(const GameState& gs, const array<Card, 8>& picks, array<int, 8>& curve)
    {
        const int possiblePickCount = 3;
        double maxValue = -10000;
        int bestPickIndex = 0;
        for(int i = 0; i < possiblePickCount; i++)
        {
            double cardValue = GetValue(gs, picks[i], curve);
            cerr << "card " << i << " value: " << cardValue << endl;
            if(cardValue >= maxValue)
            {
                maxValue = cardValue;
                bestPickIndex = i;
            }
        }
        if(picks[bestPickIndex].Type == CardType::Creature)
            CurveAdd(picks[bestPickIndex].Cost, curve);
        stringstream ss;
        ss << "PICK " << bestPickIndex;
        return ss.str();
    }
};

namespace BattlePhase
{
    class ActionPool
    {
        vector<shared_ptr<GameAction>> pool;
    public:
        bool HasAction(const GameAction& action)
        {
            return std::any_of(pool.begin(), pool.end(), [&](auto& a)
            {
                return *a == action;
            });
        }

        void Register(const GameAction& a)
        {
            pool.emplace_back(make_shared<GameAction>(a));
        }

        shared_ptr<GameAction>& GetAction(const GameAction& action)
        {
            for(auto& a : pool)
            {
                if(*a == action)
                    return a;
            }
            throw std::runtime_error("GetAction was called with bad action!");
        }

        auto size()
        {
            return pool.size();
        }
    };

    struct GraphSolver
    {
        static string ProcessTurn(const GameState& gs, long timeout)
        {
            ActionSequence seq = DecideOnBestActionSequence(gs, timeout);
            return seq.ToString();
        }

        static ActionSequence DecideOnBestActionSequence(const GameState& initialGameSate, 
            long timeout)
        {
            double bestValue = -100000000.0;
            ActionSequence bestSeq;
            auto possibleStates = std::queue<pair<GameState, ActionSequence>>();
            possibleStates.emplace(initialGameSate, bestSeq);
            
            Stopwatch sw;
            sw.Restart();

            int counter = 0;
            int bestCounter = 0;
            while(!possibleStates.empty())
            {
                counter++;
                const auto state = possibleStates.front();
                possibleStates.pop();
                const GameState& gs = state.first;
                const ActionSequence& toState = state.second;

                if(gs.EnemyPlayer.Health <= 0)
                {
                    printError("GraphSolver found winning sequence");
                    bestSeq = toState;
                    break;
                }

                const double value = EvaluateGameState(gs);
                if(value > bestValue)
                {
#ifdef CCGDeveloper0
                    cerr << "GraphSolver NEW best value found: " << value << " with action " << toState.ToString() << endl;
                    PrintEvaluateGameState(gs);
#endif
                    bestSeq = toState;
                    bestValue = value;
                    bestCounter = counter;
                }

                auto actions = GetPossibleActions(gs);

                for(auto& a : actions)
                {
                    GameState actionGameState = Simulator::SimulateAction(gs, a);
                    possibleStates.emplace(actionGameState, toState.Extended(a));
                }

                //if (counter % 200 == 0)
                //{
                //    cerr << "GraphSolver elapsed time: " << sw.ElapsedMilliseconds << " ms";
                //}

#ifndef CCGDeveloper0
                if(sw.ElapsedMilliseconds() > timeout)
                {
                    printError("GraphSolver took to much time, breaking out");
                    break;
                }
#endif
            }

            auto elapsed = sw.ElapsedMilliseconds();
            cerr << "GraphSolver finished in " << elapsed << " ms with " << counter << " nodes" << endl;
            cerr << "GraphSolver Chosen action has index: " << bestCounter << ", has value: " << bestValue << " , is " << bestSeq.ToString() << endl;
            return bestSeq;
        }

        static vector<GameAction> GetPossibleActions(const GameState& gs)
        {
            /* Possible actions are:
            * - attacking with creature to
            *      -player
            *      -other creature
            * - play card
            *      - summon creature
            *      - use item
            *
            * */
            vector<GameAction> result;

            bool enemyHasGuard = std::any_of(gs.EnemyBoard.begin(), gs.EnemyBoard.begin()+gs.EnemyBoardCount, [](auto& c){return c.HasGuard();});
            if(!enemyHasGuard)
            {
                for(auto i = 0; i < gs.MyBoardCount; i++)
                {
                    auto& c = gs.MyBoard[i];
                    if(c.GetDidAttack() || c.GetJustSummoned())
                        continue;
                    result.emplace_back(GameActionFactory::CreatureAttack(c.InstanceId, GameAction::EnemyPlayerId));
                    
                    for(auto j = 0; j < gs.EnemyBoardCount; j++)
                    {
                        auto& e = gs.EnemyBoard[j];
                        result.emplace_back(GameActionFactory::CreatureAttack(c.InstanceId, e.InstanceId));
                    }
                }
            }
            else
            {
                for(auto i = 0; i < gs.MyBoardCount; i++)
                {
                    auto& c = gs.MyBoard[i];
                    if(c.GetDidAttack() || c.GetJustSummoned())
                        continue;                 
                    for(auto j = 0; j < gs.EnemyBoardCount; j++)
                    {
                        auto& e = gs.EnemyBoard[j];
                        if(e.HasGuard())
                            result.emplace_back(GameActionFactory::CreatureAttack(c.InstanceId, e.InstanceId));
                    }
                }
            }
            
            for(auto i = 0; i < gs.MyHandCount; i++)
            {
                auto& c = gs.MyHand[i];
                if(c.Cost > gs.MyPlayer.Mana)
                    continue;

                if(c.Type == CardType::Creature)
                {
                    if(gs.MyBoardCount < 6)
                        result.emplace_back(GameActionFactory::SummonCreature(c.InstanceId));
                }
                else if(c.Type == CardType::GreenItem)
                {
                    for(auto j = 0; j < gs.MyBoardCount; j++)
                    {
                        auto& b = gs.MyBoard[j];
                        result.emplace_back(GameActionFactory::UseItem(c.InstanceId, b.InstanceId));
                    }
                }
                else if(c.Type == CardType::RedItem)
                {
                    for(auto j = 0; j < gs.EnemyBoardCount; j++)
                    {
                        auto& b = gs.EnemyBoard[j];
                        result.emplace_back(GameActionFactory::UseItem(c.InstanceId, b.InstanceId));
                    }
                }
                else if(c.Type == CardType::BlueItem)
                {
                    result.emplace_back(GameActionFactory::UseItem(c.InstanceId, GameAction::EnemyPlayerId));
                    if(c.DefenseValue != 0)
                    {
                        for(auto j = 0; j < gs.EnemyBoardCount; j++)
                        {
                            auto& b = gs.EnemyBoard[j];
                            result.emplace_back(GameActionFactory::UseItem(c.InstanceId, b.InstanceId));
                        }
                    }
                }
            }

            return result;
        }

        static double EvaluateGameState(const GameState& gs)
        {
            // TODO: Better evaluation function
            // An evaluation function is the hardest and most important part of an AI
            
            const auto damageGatherer = [](const double s, auto& c) { return s + (double)c.AttackValue;};
            const auto hpGatherer = [](const double s, auto& c) { 
                double guardDef = c.HasGuard() ? c.DefenseValue : 0.0;
                double wardCoeff = c.HasWard() ? 2.0 : 1.0;
                return s + guardDef*wardCoeff;
            };


            double myDamage = std::accumulate(gs.MyBoard.begin(), gs.MyBoardEnd(), 0.0, damageGatherer);
            double myCards = 0.8*gs.MyHandCount;
            double myPotential = fmax(0.5, myDamage+myCards);
            double enemyHp = gs.EnemyPlayer.Health + std::accumulate(gs.EnemyBoard.begin(), gs.EnemyBoardEnd(), 0.0, hpGatherer);
            double myTurnsToWin = enemyHp / myPotential;
            
            double enemyDamage = std::accumulate(gs.EnemyBoard.begin(), gs.EnemyBoardEnd(), 0.0, damageGatherer);
            double enemyCards = 0.8*gs.EnemyHandCount;
            double enemyPotential = fmax(0.5, enemyDamage+enemyCards);
            double myHp = gs.MyPlayer.Health + std::accumulate(gs.MyBoard.begin(), gs.MyBoardEnd(), 0.0, hpGatherer);
            double enemyTurnsToWin = myHp / enemyPotential;

            double result = enemyTurnsToWin / myTurnsToWin;

            /*result += gs.MyPlayer.Health;
            result -= gs.EnemyPlayer.Health;
            result += gs.MyBoardCount;

            const auto valueGatherer = [](const double s, auto c)
            {
				const double hp = (double)c.AttackValue + (double)c.DefenseValue;
				const double guard = c.HasGuard() ? (double)c.DefenseValue : 0.0;
				return s + hp + guard;
            };
            result += std::accumulate(gs.MyBoard.begin(), gs.MyBoard.begin()+gs.MyBoardCount, 0.0, valueGatherer);
            result -= std::accumulate(gs.EnemyBoard.begin(), gs.EnemyBoard.begin()+gs.EnemyBoardCount, 0.0, valueGatherer);*/
            return result;
        }
        

        static void PrintEvaluateGameState(const GameState& gs)
        {
            // TODO: Better evaluation function
            // An evaluation function is the hardest and most important part of an AI
            const auto damageGatherer = [](const double s, auto& c) { return s + (double)c.AttackValue;};
            const auto hpGatherer = [](const double s, auto& c) { 
                double guardDef = c.HasGuard() ? c.DefenseValue : 0.0;
                double wardCoeff = c.HasWard() ? 2.0 : 1.0;
                return s + guardDef*wardCoeff;
            };


            double myDamage = std::accumulate(gs.MyBoard.begin(), gs.MyBoardEnd(), 0.0, damageGatherer);
            double myCards = 0.8*gs.MyHandCount;
            double myPotential = fmax(0.5, myDamage+myCards);
            double enemyHp = gs.EnemyPlayer.Health + std::accumulate(gs.EnemyBoard.begin(), gs.EnemyBoardEnd(), 0.0, hpGatherer);
            double myTurnsToWin = enemyHp / myPotential;
            
            double enemyDamage = std::accumulate(gs.EnemyBoard.begin(), gs.EnemyBoardEnd(), 0.0, damageGatherer);
            double enemyCards = 0.8*gs.EnemyHandCount;
            double enemyPotential = fmax(0.5, enemyDamage+enemyCards);
            double myHp = gs.MyPlayer.Health + std::accumulate(gs.MyBoard.begin(), gs.MyBoardEnd(), 0.0, hpGatherer);
            double enemyTurnsToWin = myHp / enemyPotential;

            double result = enemyTurnsToWin - myTurnsToWin;

            cerr << "****GameState evaluate****" << endl;
            cerr << "myDamage: " << myDamage << endl;
            cerr << "myCards: " << myCards << endl;
            cerr << "myPotential: " << myPotential << endl;
            cerr << "enemyHp: " << enemyHp << endl;
            cerr << "myTurnsToWin: " << myTurnsToWin << endl;
            
            cerr << "--------------" << endl;
            
            cerr << "enemyDamage: " << enemyDamage << endl;
            cerr << "enemyCards: " << enemyCards << endl;
            cerr << "enemyPotential: " << enemyPotential << endl;
            cerr << "myHp: " << myHp << endl;
            cerr << "enemyTurnsToWin: " << enemyTurnsToWin << endl;
            
            cerr << "result: " << result << endl << endl;

        }
    };
}

struct Parse
{
    static CCG::Gambler Gambler(const string& input)
    {
        std::vector<std::string> inputs = Utils::split(input, ' ');
        CCG::Gambler gambler;
        gambler.Health = stoi(inputs[0]),
        gambler.Mana = stoi(inputs[1]),
        gambler.DeckSize = stoi(inputs[2]),
        gambler.NextRuneThreshold = stoi(inputs[3]);
        return gambler;
    }

    static CardAbility Ability(const string& abilities)
    {
        const auto nothing = CardAbility::Nothing;
		auto result = nothing;
        const auto hasChar = [&](const char c) -> bool {
            return abilities.find(c) != string::npos;
        };
        result |= (hasChar('B') ? CardAbility::Breakthrough : nothing);
        result |= (hasChar('C') ? CardAbility::Charge : nothing);
        result |= (hasChar('D') ? CardAbility::Drain : nothing);
        result |= (hasChar('G') ? CardAbility::Guard : nothing);
        result |= (hasChar('L') ? CardAbility::Lethal : nothing);
        result |= (hasChar('W') ? CardAbility::Ward : nothing);
        return result;
    }

    static CCG::Card Card(const string& input)
    {
        std::vector<std::string> inputs = Utils::split(input, ' ');
        CCG::Card card;
        card.CardNumber = stoi(inputs[0]);
        card.InstanceId = stoi(inputs[1]);
        card.Location = static_cast<BoardLocation>(stoi(inputs[2]));
        card.Type = static_cast<CardType>(stoi(inputs[3]));
        card.Cost = stoi(inputs[4]);
        card.AttackValue = stoi(inputs[5]);
        card.DefenseValue = stoi(inputs[6]);
        card.Abilities = Parse::Ability(inputs[7]);
        card.MyHealthChange = stoi(inputs[8]);
        card.EnemyHealthChange = stoi(inputs[9]);
        card.CardDraw = stoi(inputs[10]);
        return card;
    }

    static void AddCardByLocation(const CCG::Card& card, CCG::GameState& gs)
    {
        switch(card.Location)
        {
            case BoardLocation::EnemySide:
                gs.AddCardToEnemyBoard(card);
                break;
            case BoardLocation::InHand:
                gs.AddCardToMyHand(card);
                break;
            case BoardLocation::PlayerSide:
                gs.AddCardToMyBoard(card);
                break;
        }
    }

    static CCG::GameState GameStateFromConsole()
    {
        CCG::GameState gs;
        string line;
        std::getline(std::cin, line);
        gs.MyPlayer = Parse::Gambler(line);
        std::getline(std::cin, line);
        gs.EnemyPlayer = Parse::Gambler(line);
		std::getline(std::cin, line);
        gs.EnemyHandCount = static_cast<char>(stoi(line));
		std::getline(std::cin, line);
        gs.CardCount = static_cast<char>(stoi(line));
		
        for(auto i = 0; i < gs.CardCount; i++)
        {
            std::getline(std::cin, line);
            CCG::Card card = Parse::Card(line);
            AddCardByLocation(card, gs);
        }
        return gs;
    }

    static CCG::GameState GameState(queue<string> lines)
    {
        CCG::GameState gs;
        gs.MyPlayer = Parse::Gambler(lines.front()); lines.pop();
        gs.EnemyPlayer = Parse::Gambler(lines.front()); lines.pop();
        gs.EnemyHandCount = stoi(lines.front()); lines.pop();
        gs.CardCount = stoi(lines.front()); lines.pop();

        for(int i = 0; i < gs.CardCount; i++)
        {
            CCG::Card card = Parse::Card(lines.front()); lines.pop();
            AddCardByLocation(card, gs);
        }
        return gs;
    }
};

}




/*
//////////Test:
9 7 16 5
33 7 19 25
4
15
75 10 0 0 5 6 5 B----- 0 0 0
50 14 0 0 3 3 2 ----L- 0 0 0
23 16 0 0 7 8 8 ------ 0 0 0
100 20 0 0 3 1 6 ---G-- 0 0 0
99 22 0 0 3 2 5 ---G-- 0 0 0
99 24 0 0 3 2 5 ---G-- 0 0 0
93 26 0 0 1 2 1 ---G-- 0 0 0
75 28 0 0 5 6 5 B----- 0 0 0
17 4 1 0 4 4 3 ------ 0 0 0
10 6 1 0 3 3 1 --D--- 0 0 0
69 8 1 0 3 4 4 B----- 0 0 0
69 3 -1 0 3 4 1 B----- 0 0 0
1 17 -1 0 1 2 1 ------ 0 0 0
76 1 -1 0 6 5 5 B-D--- 0 0 0
45 9 -1 0 6 6 5 B-D--- -3 0 0

                "9 7 16 5", "33 7 19 25", "4", "15",
                "75 10 0 0 5 6 5 B----- 0 0 0",
                "50 14 0 0 3 3 2 ----L- 0 0 0",
                "23 16 0 0 7 8 8 ------ 0 0 0",
                "100 20 0 0 3 1 6 ---G-- 0 0 0",
                "99 22 0 0 3 2 5 ---G-- 0 0 0",
                "99 24 0 0 3 2 5 ---G-- 0 0 0",
                "93 26 0 0 1 2 1 ---G-- 0 0 0",
                "75 28 0 0 5 6 5 B----- 0 0 0",
                "17 4 1 0 4 4 3 ------ 0 0 0",
                "10 6 1 0 3 3 1 --D--- 0 0 0",
                "69 8 1 0 3 4 4 B----- 0 0 0",
                "69 3 -1 0 3 4 1 B----- 0 0 0",
                "1 17 -1 0 1 2 1 ------ 0 0 0",
                "76 1 -1 0 6 5 5 B-D--- 0 0 0",
                "45 9 -1 0 6 6 5 B-D--- -3 0 0"

/////////////Error: SUMMON 3;Attack 5 -1
28 3 23 25
32 2 23 25
5
9
19 1 0 0 5 5 6 ------ 0 0 0
86 3 0 0 3 1 5 -C---- 0 0 0
87 5 0 0 4 2 5 -C-G-- 0 0 0
22 7 0 0 6 7 5 ------ 0 0 0
32 9 0 0 3 3 2 ------ 0 0 1
122 11 0 1 2 1 3 ---G-- 0 0 0
27 13 0 0 2 2 2 ------ 2 0 0
3 8 -1 0 1 2 2 ------ 0 0 0
27 4 -1 0 2 2 2 ------ 2 0 0

                "28 3 23 25", "32 2 23 25", "5", "9",
                "19 1 0 0 5 5 6 ------ 0 0 0",
                "86 3 0 0 3 1 5 -C---- 0 0 0",
                "87 5 0 0 4 2 5 -C-G-- 0 0 0",
                "22 7 0 0 6 7 5 ------ 0 0 0",
                "32 9 0 0 3 3 2 ------ 0 0 1",
                "122 11 0 1 2 1 3 ---G-- 0 0 0",
                "27 13 0 0 2 2 2 ------ 2 0 0",
                "3 8 -1 0 1 2 2 ------ 0 0 0",
                "27 4 -1 0 2 2 2 ------ 2 0 0"

// PASS ing here?
32 4 22 25
31 3 22 25
6
8
18 1 0 0 4 7 4 ------ 0 0 0
11 3 0 0 3 5 2 ------ 0 0 0
47 5 0 0 2 1 5 --D--- 0 0 0
15 9 0 0 4 4 5 ------ 0 0 0
99 11 0 0 3 2 5 ---G-- 0 0 0
73 13 0 0 4 4 4 B----- 4 0 0
3 15 0 0 1 2 2 ------ 0 0 0
41 14 -1 0 3 3 1 -CD--- 0 0 0

				"32 4 22 25", "31 3 22 25", "6", "8",
				"18 1 0 0 4 7 4 ------ 0 0 0",
				"11 3 0 0 3 5 2 ------ 0 0 0",
				"47 5 0 0 2 1 5 --D--- 0 0 0",
				"15 9 0 0 4 4 5 ------ 0 0 0",
				"99 11 0 0 3 2 5 ---G-- 0 0 0",
				"73 13 0 0 4 4 4 B----- 4 0 0",
				"3 15 0 0 1 2 2 ------ 0 0 0",
				"41 14 -1 0 3 3 1 -CD--- 0 0 0"

///////////////questionable item use
29 5 19 25
28 5 20 25
6
10
79 10 0 0 8 8 8 B----- 0 0 0
-105 12 0 2 5 0 -99 BCDGLW 0 0 0
75 14 0 0 5 6 5 B----- 0 0 0
72 16 0 0 4 5 3 B----- 0 0 0
99 18 0 0 3 2 5 ---G-- 0 0 0
73 20 0 0 4 4 4 B----- 4 0 0
-105 22 0 2 5 0 -99 BCDGLW 0 0 0
27 4 1 0 2 2 2 ------ 0 0 0
73 15 -1 0 4 4 2 B----- 0 0 0
75 9 -1 0 5 6 5 B----- 0 0 0

				"29 5 19 25", "28 5 20 25", "6", "10",
				"79 10 0 0 8 8 8 B----- 0 0 0",
				"-105 12 0 2 5 0 -99 BCDGLW 0 0 0",
				"75 14 0 0 5 6 5 B----- 0 0 0",
				"72 16 0 0 4 5 3 B----- 0 0 0",
				"99 18 0 0 3 2 5 ---G-- 0 0 0",
				"73 20 0 0 4 4 4 B----- 4 0 0",
				"-105 22 0 2 5 0 -99 BCDGLW 0 0 0",
				"27 4 1 0 2 2 2 ------ 0 0 0",
				"73 15 -1 0 4 4 2 B----- 0 0 0",
				"75 9 -1 0 5 6 5 B----- 0 0 0"

//////////////// Timeout test
23 9 15 20
23 8 16 20
5
15
23 1 0 0 7 8 8 ------ 0 0 0
77 11 0 0 7 7 7 B----- 0 0 0
114 13 0 0 7 7 7 ---G-- 0 0 0
74 21 0 0 5 5 4 B--G-- 0 0 0
69 23 0 0 3 4 4 B----- 0 0 0
63 27 0 0 2 0 4 ---G-W 0 0 0
99 29 0 0 3 2 5 ---G-- 0 0 0
86 7 1 0 3 1 5 -C---- 0 0 0
1 15 1 0 1 2 1 ------ 0 0 0
37 3 1 0 6 5 5 ------ 0 0 0
75 19 1 0 5 7 6 B----- 0 0 0
58 22 -1 0 6 5 6 B----- 0 0 0
69 4 -1 0 3 4 4 B----- 0 0 0
75 28 -1 0 5 6 5 B----- 0 0 0
69 20 -1 0 3 4 4 B----- 0 0 0

                "23 9 15 20", "23 8 16 20", "5", "15",
                "23 1 0 0 7 8 8 ------ 0 0 0",
                "77 11 0 0 7 7 7 B----- 0 0 0",
                "114 13 0 0 7 7 7 ---G-- 0 0 0",
                "74 21 0 0 5 5 4 B--G-- 0 0 0",
                "69 23 0 0 3 4 4 B----- 0 0 0",
                "63 27 0 0 2 0 4 ---G-W 0 0 0",
                "99 29 0 0 3 2 5 ---G-- 0 0 0",
                "86 7 1 0 3 1 5 -C---- 0 0 0",
                "1 15 1 0 1 2 1 ------ 0 0 0",
                "37 3 1 0 6 5 5 ------ 0 0 0",
                "75 19 1 0 5 7 6 B----- 0 0 0",
                "58 22 -1 0 6 5 6 B----- 0 0 0",
                "69 4 -1 0 3 4 4 B----- 0 0 0",
                "75 28 -1 0 5 6 5 B----- 0 0 0",
                "69 20 -1 0 3 4 4 B----- 0 0 0"

Server:
GraphSolver finished in 91 ms with 1310 nodes
GraphSolver Chosen action has index: 696, has value: 11 , is ATTACK 7 -1;ATTACK 3 -1;ATTACK 19 -1

Release:
GraphSolver finished in 14491 ms with 5854966 nodes
GraphSolver Chosen action has index: 3826097, has value: 17 , is ATTACK 7 -1;ATTACK 15 28;ATTACK 3 20;ATTACK 19 28;SUMMON 23;SUMMON 27;SUMMON 29


14 7 15 10
15 7 16 10
5
17
27 8 0 0 2 2 2 ------ 2 0 0
116 16 0 0 12 8 8 BCDGLW 0 0 0
7 20 0 0 2 2 2 -----W 0 0 0
116 22 0 0 12 8 8 BCDGLW 0 0 0
41 24 0 0 3 2 2 -CD--- 0 0 0
72 26 0 0 4 5 3 B----- 0 0 0
26 28 0 0 2 3 2 ------ 0 -1 0
19 30 0 0 5 5 6 ------ 0 0 0
50 10 1 0 3 3 2 ----L- 0 0 0
26 18 1 0 2 3 2 ------ 0 0 0
37 14 1 0 6 5 7 ------ 0 0 1
48 13 -1 0 1 1 1 ----L- 0 0 0
8 17 -1 0 2 2 3 ------ 0 0 0
72 11 -1 0 4 5 3 B----- 0 0 0
7 19 -1 0 2 2 2 -----W 0 0 0
37 5 -1 0 6 5 7 ------ 0 0 1
39 23 -1 0 1 2 1 --D--- 0 0 0

                "14 7 15 10", "15 7 16 10", "5", "17",
                "27 8 0 0 2 2 2 ------ 2 0 0",
                "116 16 0 0 12 8 8 BCDGLW 0 0 0",
                "7 20 0 0 2 2 2 -----W 0 0 0",
                "116 22 0 0 12 8 8 BCDGLW 0 0 0",
                "41 24 0 0 3 2 2 -CD--- 0 0 0",
                "72 26 0 0 4 5 3 B----- 0 0 0",
                "26 28 0 0 2 3 2 ------ 0 -1 0",
                "19 30 0 0 5 5 6 ------ 0 0 0",
                "50 10 1 0 3 3 2 ----L- 0 0 0",
                "26 18 1 0 2 3 2 ------ 0 0 0",
                "37 14 1 0 6 5 7 ------ 0 0 1",
                "48 13 -1 0 1 1 1 ----L- 0 0 0",
                "8 17 -1 0 2 2 3 ------ 0 0 0",
                "72 11 -1 0 4 5 3 B----- 0 0 0",
                "7 19 -1 0 2 2 2 -----W 0 0 0",
                "37 5 -1 0 6 5 7 ------ 0 0 1",
                "39 23 -1 0 1 2 1 --D--- 0 0 0",


/////////////////////// ATTACK 22 43;ATTACK 40 -1 starting wouls be strictly better
30 12 5 0
13 12 6 0
3
7
72 44 0 0 4 5 3 B----- 0 0 0
19 46 0 0 5 5 6 ------ 0 0 0
47 48 0 0 2 1 5 --D--- 0 0 0
117 50 0 1 1 1 1 B----- 0 0 0
9 40 1 0 3 3 1 ------ 0 0 0
116 22 1 0 12 8 8 BCDGLW 0 0 0
116 43 -1 0 12 8 8 BCDGL- 0 0 0

                "30 12 5 0", "13 12 6 0", "3", "7",
                "72 44 0 0 4 5 3 B----- 0 0 0",
                "19 46 0 0 5 5 6 ------ 0 0 0",
                "47 48 0 0 2 1 5 --D--- 0 0 0",
                "117 50 0 1 1 1 1 B----- 0 0 0",
                "9 40 1 0 3 3 1 ------ 0 0 0",
                "116 22 1 0 12 8 8 BCDGLW 0 0 0",
                "116 43 -1 0 12 8 8 BCDGL- 0 0 0"

// timeout
43 12 10 20
19 11 10 15
4
11
34 7 0 0 5 3 5 ------ 0 0 1
116 13 0 0 12 8 8 BCDGLW 0 0 0
44 27 0 0 6 3 7 --D-L- 0 0 0
42 37 0 0 4 4 2 --D--- 0 0 0
-105 39 0 2 5 0 -99 BCDGLW 0 0 0
76 31 1 0 6 5 2 B-D--- 0 0 0
82 1 1 0 7 5 5 B-D--- 0 0 0
40 35 1 0 3 2 3 --DG-- 0 0 0
42 33 1 0 4 4 2 --D--- 0 0 0
21 36 -1 0 5 6 5 ------ 0 0 0
16 40 -1 0 4 6 2 ------ 0 0 0

				"43 12 10 20",
				"19 11 10 15",
				"4",
				"11",
				"34 7 0 0 5 3 5 ------ 0 0 1",
				"116 13 0 0 12 8 8 BCDGLW 0 0 0",
				"44 27 0 0 6 3 7 --D-L- 0 0 0",
				"42 37 0 0 4 4 2 --D--- 0 0 0",
				"-105 39 0 2 5 0 -99 BCDGLW 0 0 0",
				"76 31 1 0 6 5 2 B-D--- 0 0 0",
				"82 1 1 0 7 5 5 B-D--- 0 0 0",
				"40 35 1 0 3 2 3 --DG-- 0 0 0",
				"42 33 1 0 4 4 2 --D--- 0 0 0",
				"21 36 -1 0 5 6 5 ------ 0 0 0",
				"16 40 -1 0 4 6 2 ------ 0 0 0"
*/

void mainTest()
{
	cerr << "CardSize: " << sizeof(CCG::Card) << endl;

    CCG::Stopwatch sw;
    // game loop
    //while (true)
    {
        sw.Restart();
        CCG::GameState gs = CCG::Parse::GameState(std::queue<string>({
			"43 12 10 20",
			"19 11 10 15",
			"4",
			"11",
			"34 7 0 0 5 3 5 ------ 0 0 1",
			"116 13 0 0 12 8 8 BCDGLW 0 0 0",
			"44 27 0 0 6 3 7 --D-L- 0 0 0",
			"42 37 0 0 4 4 2 --D--- 0 0 0",
			"-105 39 0 2 5 0 -99 BCDGLW 0 0 0",
			"76 31 1 0 6 5 2 B-D--- 0 0 0",
			"82 1 1 0 7 5 5 B-D--- 0 0 0",
			"40 35 1 0 3 2 3 --DG-- 0 0 0",
			"42 33 1 0 4 4 2 --D--- 0 0 0",
			"21 36 -1 0 5 6 5 ------ 0 0 0",
			"16 40 -1 0 4 6 2 ------ 0 0 0"
            }));
        
        CCG::printError(gs.ToString());

        cout << CCG::BattlePhase::GraphSolver::ProcessTurn(gs, 9500) << endl;

        cerr << "Turn took " << sw.ElapsedMilliseconds() << " ms" << endl;
    }
}

array<int, 8> curve = { 2, 3, 6, 5, 4, 3, 2, 2 };

void mainTestDraft()
{
	cerr << "CardSize: " << sizeof(CCG::Card) << endl;

    CCG::Stopwatch sw;
    // game loop
    //while (true)
    {
        sw.Restart();
        CCG::GameState gs = CCG::Parse::GameState(std::queue<string>({
			"30 0 3 25", "30 0 4 25", "0", "3",
            "71 -1 0 0 4 3 2 BC---- 0 0 0",
            "-117 -1 0 1 4 0 0 ----LW 0 0 0",
            "9 -1 0 0 3 3 4 ------ 0 0 0"
            }));
        
        CCG::printError(gs.ToString());

        cout << CCG::DraftPhase::GetBestCard(gs, gs.MyHand, curve) << endl;

        cerr << "Turn took " << sw.ElapsedMilliseconds() << " ms" << endl;
    }
}

int mainReal()
{
    int turn = 0;

    const int draftTurnCount = 30;
    const int lastTurn = draftTurnCount + 50;

    // game loop
    while(true)
    {
        CCG::GameState gs = CCG::Parse::GameStateFromConsole();
        cerr << gs.ToString() << endl;

        if(turn < lastTurn)
        {
            ++turn;
        }

        if(turn <= draftTurnCount)
        {
            cout << CCG::DraftPhase::GetBestCard(gs, gs.MyHand, curve) << endl;
        }
        else
        {
            cout << CCG::BattlePhase::GraphSolver::ProcessTurn(gs, 92) << endl;
        }
    }
}

int main()
{
#if defined(CCGDeveloper)
    mainTestDraft();
    mainTest();
#else
    mainReal();
#endif
    return 0;
}