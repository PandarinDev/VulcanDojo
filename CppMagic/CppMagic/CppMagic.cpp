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

using namespace std;

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
    bool vecEqual(const vector<T> v1, const vector<T> v2)
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

    template<class T, int Size>
    bool arrayEqual(const std::array<T, Size> v1, const std::array<T, Size> v2)
    {
        for(size_t i = 0; i < v1.size(); i++)
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

    auto join = [](auto collection, auto separator) -> auto
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

    auto joinSize = [](auto collection, auto size, auto separator) -> auto
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
    string toString(vector<T> cards)
    {
        string r = Utils::join(cards, '\n');
        return r;
    }
    
    template<class T, int S>
    string toString(array<T, S> cards, char size)
    {
        string r = Utils::joinSize(cards, size, '\n');
        return r;
    }
};

namespace CCG
{

enum BoardLocation
{
    EnemySide = -1,
    InHand = 0,
    PlayerSide = 1,
    PlayerSidePassive = 2
};

enum CardType
{
    Creature = 0,
    GreenItem = 1,
    RedItem = 2,
    BlueItem = 3
};

enum CardAbility
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

    bool DidAttack = false;
    
    Card() = default;
    Card(const Card&) = default;
    Card(Card&&) = default;
    Card& operator=(const Card&) = default;

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
        Abilities = (CardAbility)(Abilities | a);
    }

    void RemoveBreakthrough() { RemoveAbilty(CardAbility::Breakthrough); }
    void RemoveCharge() { RemoveAbilty(CardAbility::Charge);}
    void RemoveDrain() { RemoveAbilty(CardAbility::Drain);}
    void RemoveGuard() { RemoveAbilty(CardAbility::Guard);}
    void RemoveLethal() { RemoveAbilty(CardAbility::Lethal);}
    void RemoveWard() { RemoveAbilty(CardAbility::Ward);}
    void RemoveAbilty(CardAbility a) {
        Abilities = (CardAbility)((Abilities & ~a) & CardAbility::All);
    }

    string ToString() const
    {
        stringstream ss;
        ss << (int)CardNumber << " " << (int)InstanceId << " " << Location << " " << Type << " " << (int)Cost << " " << (int)AttackValue << " " << (int)DefenseValue << " " << AbilitiesToString() << " " << (int)MyHealthChange << " " << (int)EnemyHealthChange << " " << (int)CardDraw;
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
        auto result = CardNumber == c.CardNumber && InstanceId == c.InstanceId &&
            Location == c.Location && Type == c.Type &&
            Cost == c.Cost && AttackValue == c.AttackValue &&
            DefenseValue == c.DefenseValue && Abilities == c.Abilities &&
            MyHealthChange == c.MyHealthChange && EnemyHealthChange == c.EnemyHealthChange &&
            CardDraw == c.CardDraw;
        return result;
    }

    bool operator==(const Card& c)
    {
		auto result = CardNumber == c.CardNumber && InstanceId == c.InstanceId &&
            Location == c.Location && Type == c.Type &&
            Cost == c.Cost && AttackValue == c.AttackValue &&
            DefenseValue == c.DefenseValue && Abilities == c.Abilities &&
            MyHealthChange == c.MyHealthChange && EnemyHealthChange == c.EnemyHealthChange &&
            CardDraw == c.CardDraw;
        return result;
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

string toString(const shared_ptr<GameAction>& a)
{
    return a->ToString();
}

struct ActionSequence
{
    vector<shared_ptr<GameAction>> Actions;

    string ToString()
    {
        string actions = Utils::join(Actions, ';');
        return (actions == "") ? "PASS" : actions;
    }
    
    ActionSequence() = default;
    ActionSequence(const ActionSequence&) = default;
    ActionSequence& operator=(const ActionSequence&) = default;

    ActionSequence Extended(const shared_ptr<GameAction>& a) const
    {
        ActionSequence copy(*this);
        copy.Add(a);
        return copy;
    }

    void Add(const shared_ptr<GameAction>& a)
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
    int EnemyHandCount;
    int CardCount; // Cards in my hand + on the board
    vector<Card> AllCards;
    
    array<Card, 8> MyHand;
    array<Card, 6> MyBoard;
    array<Card, 6> EnemyBoard;
    array<Card, 6> PassiveCards;

    char MyHandCount = 0;
    char MyBoardCount = 0;
    char EnemyBoardCount = 0;
    char PassiveCardsCount = 0;

    /*vector<Card> MyHand;
    vector<Card> MyBoard;
    vector<Card> EnemyBoard;
    vector<Card> PassiveCards;*/
    
    GameState() = default;
    GameState(const GameState&) = default;
    GameState(GameState&&) = default;
    GameState& operator=(const GameState&) = default;

    string ToString() const
    {
        vector<string> a{
            Utils::toString(MyHand, MyHandCount), 
            Utils::toString(MyBoard, MyBoardCount), 
            Utils::toString(EnemyBoard, EnemyBoardCount), 
            Utils::toString(PassiveCards, PassiveCardsCount)
        };
        string cards = Utils::toString(a);
        stringstream ss;
        ss << MyPlayer.ToString() << endl << EnemyPlayer.ToString() << endl << EnemyHandCount << endl << CardCount << endl << cards;
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
            Utils::arrayEqual(EnemyBoard, c.EnemyBoard) &&
            Utils::arrayEqual(PassiveCards, c.PassiveCards);
        return result;
    }

    void AddCardToMyHand(const Card& c) { AddCardToArray(MyHand, MyHandCount, c); }
    void AddCardToMyBoard(const Card& c) { AddCardToArray(MyBoard, MyBoardCount, c); }
    void AddCardToEnemyBoard(const Card& c) { AddCardToArray(EnemyBoard, EnemyBoardCount, c); }
    void AddCardToPassiveCards(const Card& c) { AddCardToArray(PassiveCards, PassiveCardsCount, c); }

    template<class T, int S>
    inline static void AddCardToArray(array<T, S>& arr, char& counter, const Card& c)
    {
        arr[counter] = c;
        counter++;
    }

    void RemoveCardFromMyHand(const array<Card, 8>::iterator& it) { RemoveCardFromArray(MyHand, MyHandCount, it); }
    void RemoveCardFromMyBoard(const array<Card, 6>::iterator& it) { RemoveCardFromArray(MyBoard, MyBoardCount, it); }
    void RemoveCardFromEnemyBoard(const array<Card, 6>::iterator& it) { RemoveCardFromArray(EnemyBoard, EnemyBoardCount, it); }
    void RemoveCardFromPassiveCards(const array<Card, 6>::iterator& it) { RemoveCardFromArray(PassiveCards, PassiveCardsCount, it); }
    
    static void RemoveCardFromArray(array<Card, 6>& arr, char& counter, const array<Card, 6>::iterator& it)
    {
        *it = arr[counter];
        counter--;
    }

    static void RemoveCardFromArray(array<Card, 8>& arr, char& counter, const array<Card, 8>::iterator& it)
    {
        *it = arr[counter];
        counter--;
    }
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
        attacker.DidAttack = true;
    }

    void AttackCreature(Card& attacker, Card& defender)
    {
        HalfAttack(attacker, defender);

        if(!defender.DidAttack)
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
            Drain(state.EnemyPlayer, defender, attackerHpBefore - max(static_cast<char>(0), attacker.DefenseValue));
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
            attacker.DidAttack = true;
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
        }
        else if(item.Type == CardType::RedItem ||
            (item.Type == CardType::BlueItem && targetId != GameAction::EnemyPlayerId))
        {
            auto& creature = *find_if(state.EnemyBoard.begin(), state.EnemyBoard.begin()+state.EnemyBoardCount, [=](const Card& c) { return c.InstanceId == targetId;});
            creature.RemoveAbilty(item.Abilities);
            creature.AttackValue += item.AttackValue;
            creature.DefenseValue += item.DefenseValue;
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
            state.AddCardToPassiveCards(toSummon);
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
        return state;
    }
};

namespace DraftPhase
{
    bool HaveEnoughInManaCurve(int cost, int curve[])
    {
        if(cost > 1 && cost < 7 && curve[cost - 1] <= 0)
        {
            return true;
        }
        if(cost <= 1 && curve[0] <= 0)
        {
            return true;
        }
        if(cost > 6 && curve[6] <= 0)
        {
            return true;
        }
        return false;
    }

    double GetValue(Card card, int curve[])
    {
        //TODO: improve red and blue item values
        double value = 0;
        value += abs(card.AttackValue);
        value += abs(card.DefenseValue);
        value += card.CardDraw;
        auto s = card.AbilitiesToString();
        value +=  count_if(s.begin(), s.end(), [](auto s){return s != '-';});
        value += (double)card.MyHealthChange / 3;
        value -= (double)card.EnemyHealthChange / 3;
        value -= card.Cost * 2;
        //marginal penalty
        if(card.Cost == 0 || card.AttackValue == 0)
        {
            value -= 2;
        }
        //nonboard penalty
        if(card.Type == CardType::RedItem ||
            card.Type == CardType::BlueItem)
        {
            value -= 1;
        }
        //balance penalty, nerf card "Decimate"
        if(card.CardNumber == 151)
        {
            value -= 94;
        }
        if(HaveEnoughInManaCurve(card.Cost, curve))
        {
            value -= 1;
        }

        return value;
    }

    void CurveAdd(int cost, int curve[])
    {
        int place = clamp(cost - 1, 0, 6);
        curve[place] -= 1;
    }
    /// <summary>
    /// Represent a turn in the draft phase, 
    /// basically selects the card that we should pick
    /// </summary>
    string GetBestCard(const array<Card, 8>& picks, int curve[])
    {
        const int possiblePickCount = 3;
        double maxValue = -10000;
        int bestPickIndex = 0;
        for(int i = 0; i < possiblePickCount; i++)
        {
            double cardValue = GetValue(picks[i], curve);
            if(cardValue >= maxValue)
            {
                maxValue = cardValue;
                bestPickIndex = i;
            }
        }
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

        size_t size()
        {
            return pool.size();
        }
    };

    struct GraphSolver
    {
        static string ProcessTurn(const GameState& gs, long timeout)
        {
            ActionPool pool;
            ActionSequence seq = DecideOnBestActionSequence(gs, pool, timeout);
            return seq.ToString();
        }

        static ActionSequence DecideOnBestActionSequence(const GameState& initialGameSate, 
            ActionPool& actionPool, long timeout)
        {
            double bestValue = -100000000.0;
            ActionSequence bestSeq;
            auto possibleStates = std::queue<shared_ptr<pair<GameState, ActionSequence>>>();
            possibleStates.push(make_shared<pair<GameState, ActionSequence>>(initialGameSate, bestSeq));
            
            Stopwatch sw;
            sw.Restart();

            int counter = 0;
            int bestCounter = 0;
            while(!possibleStates.empty())
            {
                counter++;
                auto state = possibleStates.front();
                possibleStates.pop();
                const GameState& gs = state->first;
                const ActionSequence& toState = state->second;

                if(gs.EnemyPlayer.Health <= 0)
                {
                    printError("GraphSolver found winning sequence");
                    bestSeq = toState;
                    break;
                }

                double value = EvaluateGameState(gs);
                if(value > bestValue)
                {
                    //cerr << "GraphSolver NEW best value found: " << value << endl;
                    bestSeq = toState;
                    bestValue = value;
                    bestCounter = counter;
                }

                auto actions = GetPossibleActions(gs);

                for(auto& a : actions)
                {
                    if(!actionPool.HasAction(a))
                    {
                        actionPool.Register(a);
                    }
                    auto& action = actionPool.GetAction(a);
                    GameState actionGameState = Simulator::SimulateAction(gs, *action);
                    possibleStates.push(make_shared<pair<GameState, ActionSequence>>(actionGameState, toState.Extended(action)));
                }

                //if (counter % 200 == 0)
                //{
                //    cerr << "GraphSolver elapsed time: " << sw.ElapsedMilliseconds << " ms";
                //}

                if(sw.ElapsedMilliseconds() > timeout)
                {
                    printError("GraphSolver took to much time, breaking out");
                    break;
                }
            }

            auto elapsed = sw.ElapsedMilliseconds();
            cerr << "GraphSolver finished in " << elapsed << " ms with " << counter << " nodes" << endl;
            cerr << "GraphSolver Chosen action has index: " << bestCounter << ", has value: " << bestValue << " , is " << bestSeq.ToString() << endl ;
            cerr << "GraphSolver action pool size: " << actionPool.size() << endl;
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
                    if(c.DidAttack)
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
                    if(c.DidAttack)
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
                    if(gs.MyBoardCount + gs.PassiveCardsCount < 6)
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
                else if(c.Type == CardType::RedItem || 
                    c.Type == CardType::BlueItem && c.DefenseValue != 0)
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
                }
            }

            return result;
        }

        static double EvaluateGameState(const GameState& gs)
        {
            // TODO: Better evaluation function
            // An evaluation function is the hardest and most important part of an AI
            double result = 0.0;
            result += gs.MyPlayer.Health;
            result -= gs.EnemyPlayer.Health;
            result += gs.MyBoard.size();
            result += gs.PassiveCards.size();
            auto valueGatherer = [](double s, auto c) {return s + (double)c.AttackValue + (double)c.DefenseValue;};
            result += std::accumulate(gs.MyBoard.begin(), gs.MyBoard.begin()+gs.MyBoardCount, 0.0, valueGatherer);
            result -= std::accumulate(gs.EnemyBoard.begin(), gs.EnemyBoard.begin()+gs.EnemyBoardCount, 0.0, valueGatherer);
            return result;
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
        auto nothing = CardAbility::Nothing;
        int result = nothing;
        auto hasChar = [&](char c) -> bool {
            return abilities.find(c) != string::npos;
        };
        result |= (hasChar('B') ? Breakthrough : nothing);
        result |= (hasChar('C') ? Charge : nothing);
        result |= (hasChar('D') ? Drain : nothing);
        result |= (hasChar('G') ? Guard : nothing);
        result |= (hasChar('L') ? Lethal : nothing);
        result |= (hasChar('W') ? Ward : nothing);
        return static_cast<CardAbility>(result);
    }

    static CCG::Card Card(const string& input)
    {
        //Console.Error.WriteLine("!parse Card: " + input);
        std::vector<std::string> inputs = Utils::split(input, ' ');
        CCG::Card card;
        card.CardNumber = stoi(inputs[0]);
        card.InstanceId = stoi(inputs[1]);
        card.Location = (BoardLocation)stoi(inputs[2]);
        card.Type = (CardType)stoi(inputs[3]);
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
            case BoardLocation::PlayerSidePassive:
                gs.AddCardToPassiveCards(card);
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
        cin >> gs.EnemyHandCount; cin.ignore();
        cin >> gs.CardCount; cin.ignore();

        for(int i = 0; i < gs.CardCount; i++)
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

int mainReal()
{
    int turn = 0;

    const int draftTurnCount = 30;
    const int lastTurn = draftTurnCount + 50;
    int curve[] = { 2, 8, 7, 5, 4, 2, 2 };

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
            cout << CCG::DraftPhase::GetBestCard(gs.MyHand, curve) << endl;
        }
        else
        {
            cout << CCG::BattlePhase::GraphSolver::ProcessTurn(gs, 95) << endl;
        }
    }
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
*/

int mainTest()
{
    int turn = 0;

    const int draftTurnCount = 30;
    const int lastTurn = draftTurnCount + 50;
    int curve[] = { 2, 8, 7, 5, 4, 2, 2 };

    CCG::Stopwatch sw;
    // game loop
    //while (true)
    {
        sw.Restart();
        CCG::GameState gs = CCG::Parse::GameState(std::queue<string>({
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
            }));
        
        CCG::printError(gs.ToString());

        cout << CCG::BattlePhase::GraphSolver::ProcessTurn(gs, 95000) << endl;

        cerr << "Turn took " << sw.ElapsedMilliseconds() << " ms" << endl;
    }
    return 0;
}

int main()
{
#if defined(CCGDeveloper)
    mainTest();
#else
    mainReal();
#endif
}