#include <iostream>
#include <chrono>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <utility>
#include <numeric>
#include <memory>

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
    
    template<class T>
    string toString(vector<T> cards)
    {
        string r = Utils::join(cards, '\n');
        return r;
    }
};


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
    int CardNumber;
    int InstanceId;
    BoardLocation Location;
    CardType CardType;
    int Cost;
    int AttackValue;
    int DefenseValue;
    CardAbility Abilities;
    int MyHealthChange;
    int EnemyHealthChange;
    int CardDraw;

    bool DidAttack;
    
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
        ss << CardNumber << " " << InstanceId << " " << Location << " " << CardType << " " << Cost << " " << AttackValue << " " << DefenseValue << " " << AbilitiesToString() << " " << MyHealthChange << " " << EnemyHealthChange << " " << CardDraw;
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
        bool result = CardNumber == c.CardNumber && InstanceId == c.InstanceId &&
            Location == c.Location && CardType == c.CardType &&
            Cost == c.Cost && AttackValue == c.AttackValue &&
            DefenseValue == c.DefenseValue && Abilities == c.Abilities &&
            MyHealthChange == c.MyHealthChange && EnemyHealthChange == c.EnemyHealthChange &&
            CardDraw == c.CardDraw;
        return result;
    }

    bool operator==(const Card& c)
    {
        bool result = CardNumber == c.CardNumber && InstanceId == c.InstanceId &&
            Location == c.Location && CardType == c.CardType &&
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
    int Id;
    int TargetId = EnemyPlayerId;

    GameAction(ActionType type) : Type(type) {}

    GameAction(ActionType type, int iid) : GameAction(type)
    {
        Id = iid;
    }

    GameAction(ActionType type, int iid, int targetId) : GameAction(type)
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
            case ActionType::SummonCreature:
                ss << "SUMMON " << Id;
            case ActionType::UseItem:
                ss << "USE " << Id << " " << TargetId;
            default:
                ss << "PASS";
        }
        return ss.str();
    }
};

string toString(GameAction* a)
{
    return a->ToString();
}

struct ActionSequence
{
    vector<GameAction*> Actions;

    string ToString()
    {
        string actions = Utils::join(Actions, ';');
        return (actions == "") ? "PASS" : actions;
    }
    
    ActionSequence() = default;
    ActionSequence(const ActionSequence&) = default;
    ActionSequence& operator=(const ActionSequence&) = default;

    ActionSequence Extended(GameAction& a)
    {
        ActionSequence copy(*this);
        copy.Add(a);
        return copy;
    }

    void Add(GameAction& a)
    {
        Actions.emplace_back(&a);
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

bool vecEqual(const vector<Card> v1, const vector<Card> v2)
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

struct GameState
{
    Gambler MyPlayer;
    Gambler EnemyPlayer;
    int EnemyHandCount;
    int CardCount; // Cards in my hand + on the board
    vector<Card> AllCards;
    vector<Card> MyHand;
    vector<Card> MyBoard;
    vector<Card> EnemyBoard;
    vector<Card> PassiveCards;
    
    GameState() = default;
    GameState(const GameState&) = default;
    GameState(GameState&&) = default;
    GameState& operator=(const GameState&) = default;

    string ToString() const
    {
        vector<string> a{Utils::toString(MyHand), Utils::toString(MyBoard), Utils::toString(EnemyBoard), Utils::toString(PassiveCards)};
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
            vecEqual(MyHand, c.MyHand) &&
            vecEqual(MyBoard, c.MyBoard) &&
            vecEqual(EnemyBoard, c.EnemyBoard) &&
            vecEqual(PassiveCards, c.PassiveCards);
        return result;
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
        auto attIndex = find_if(state.MyBoard.begin(), state.MyBoard.end(), [=](const Card& c) { return c.InstanceId == attackerId;});
        auto& attacker = *attIndex;

        if(defenderId != GameAction::EnemyPlayerId)
        {
            auto defIndex = find_if(state.EnemyBoard.begin(), state.EnemyBoard.end(), [=](const Card& c) { return c.InstanceId == defenderId;});
            auto& defender = *defIndex;
            int attackerHpBefore = attacker.DefenseValue;
            int defenderHpBefore = defender.DefenseValue;
            AttackCreature(attacker, defender);
            if(attacker.DefenseValue <= 0)
            {
                state.MyBoard.erase(attIndex);
                state.CardCount -= 1;
            }
            if(defender.DefenseValue <= 0)
            {
                state.EnemyBoard.erase(defIndex);
                state.CardCount -= 1;
                if(attacker.HasBreakthrough())
                {
                    state.EnemyPlayer.Health += defender.DefenseValue;
                }
            }
            Drain(state.MyPlayer, attacker, max(0, defenderHpBefore - max(0, defender.DefenseValue)));
            Drain(state.EnemyPlayer, defender, attackerHpBefore - max(0, attacker.DefenseValue));
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
        auto itemIndex = find_if(state.MyHand.begin(), state.MyHand.end(), [=](const Card& c) { return c.InstanceId == itemId;});
        auto& item = *itemIndex;
        state.MyPlayer.Mana -= item.Cost;
        state.MyHand.erase(itemIndex);
        state.CardCount -= 1;

        state.MyPlayer.Health += item.MyHealthChange;
        state.EnemyPlayer.Health += item.EnemyHealthChange;

        if(item.CardType == CardType::GreenItem)
        {
            auto& creature = *find_if(state.MyBoard.begin(), state.MyBoard.end(), [=](const Card& c) { return c.InstanceId == targetId;});
            creature.AddAbility(item.Abilities);
            creature.AttackValue += item.AttackValue;
            creature.DefenseValue += item.DefenseValue;
        }
        else if(item.CardType == CardType::RedItem ||
            (item.CardType == CardType::BlueItem && targetId != GameAction::EnemyPlayerId))
        {
            auto& creature = *find_if(state.EnemyBoard.begin(), state.EnemyBoard.end(), [=](const Card& c) { return c.InstanceId == targetId;});
            creature.RemoveAbilty(item.Abilities);
            creature.AttackValue += item.AttackValue;
            creature.DefenseValue += item.DefenseValue;
        }
    }

    void SummonCreatureAction(GameState& state, int creatureId)
    {
        auto toSummonIndex = find_if(state.MyHand.begin(), state.MyHand.end(), [=](const Card& c) { return c.InstanceId == creatureId;});
        auto& toSummon = *toSummonIndex;
        state.MyPlayer.Mana -= toSummon.Cost;
        state.MyHand.erase(toSummonIndex);

        if(toSummon.HasCharge())
        {
            toSummon.Location = BoardLocation::PlayerSide;
            state.MyBoard.emplace_back(toSummon);
        }
        else
        {
            toSummon.Location = BoardLocation::PlayerSidePassive;
            state.PassiveCards.emplace_back(toSummon);
        }
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
        if(card.CardType == CardType::RedItem ||
            card.CardType == CardType::BlueItem)
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
    string GetBestCard(vector<Card> picks, int curve[])
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
        return "PICK " + bestPickIndex;
    }
};

namespace BattlePhase
{
    struct GraphSolver
    {
        static string ProcessTurn(GameState gs)
        {
            ActionSequence seq = DecideOnBestActionSequence(gs);
            return seq.ToString();
        }

        static ActionSequence DecideOnBestActionSequence(GameState initialGameSate)
        {
            auto possibleStates = std::queue<shared_ptr<pair<GameState, ActionSequence>>>();

            double bestValue = -100000000.0;
            ActionSequence bestSeq;
            possibleStates.emplace(make_shared<pair<GameState, ActionSequence>>(initialGameSate, bestSeq));
            
            Stopwatch sw;
            sw.Restart();

            int counter = 0;
            while(!possibleStates.empty())
            {
                counter++;
                auto state = possibleStates.front();
                possibleStates.pop();
                GameState gs = state->first;
                ActionSequence toState = state->second;

                if(gs.EnemyPlayer.Health <= 0)
                {
                    printError("GraphSolver found winning sequence");
                    bestSeq = toState;
                    break;
                }

                double value = EvaluateGameState(gs);
                if(value > bestValue)
                {
                    cerr << "GraphSolver NEW best value found: " << value << endl;
                    bestSeq = toState;
                    bestValue = value;
                }

                auto actions = GetPossibleActions(gs);

                for(auto& action : actions)
                {
                    GameState actionGameState = Simulator::SimulateAction(gs, action);
                    possibleStates.emplace(make_shared<pair<GameState, ActionSequence>>(actionGameState, toState.Extended(action)));
                }

                //if (counter % 200 == 0)
                //{
                //    Console.Error.WriteLine($"GraphSolver elapsed time: {sw.ElapsedMilliseconds} ms");
                //}

                if(sw.ElapsedMilliseconds() > 9500)
                {
                    printError("GraphSolver took to much time, breaking out");
                    break;
                }
            }

            auto elapsed = sw.ElapsedMilliseconds();
            cerr << "GraphSolver finished in " << elapsed << " ms with " << counter << " nodes" << endl;
            cerr << "GraphSolver Chosen action has value: " << bestValue << " , is " << bestSeq.ToString() << endl ;
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

            bool enemyHasGuard = std::any_of(gs.EnemyBoard.begin(), gs.EnemyBoard.end(), [](Card c){return c.HasGuard();});
            if(!enemyHasGuard)
            {
                for(auto& c : gs.MyBoard)
                {
                    if(c.DidAttack)
                        continue;
                    result.emplace_back(GameActionFactory::CreatureAttack(c.InstanceId, GameAction::EnemyPlayerId));
                    
                    for(auto& e : gs.EnemyBoard)
                    {
                        result.emplace_back(GameActionFactory::CreatureAttack(c.InstanceId, e.InstanceId));
                    }
                }
            }
            else
            {
                for(auto& c : gs.MyBoard)
                {
                    if(c.DidAttack)
                        continue;                    
                    for(auto& e : gs.EnemyBoard)
                    {
                        if(e.HasGuard())
                            result.emplace_back(GameActionFactory::CreatureAttack(c.InstanceId, e.InstanceId));
                    }
                }
            }
            
            for(auto& c : gs.MyHand)
            {
                if(c.Cost > gs.MyPlayer.Mana)
                    continue;

                if(c.CardType == CardType::Creature)
                {
                    result.emplace_back(GameActionFactory::SummonCreature(c.InstanceId));
                }
                else if(c.CardType == CardType::GreenItem)
                {
                    for(auto& b : gs.MyBoard)
                    {
                        result.emplace_back(GameActionFactory::UseItem(c.InstanceId, b.InstanceId));
                    }
                }
                else if(c.CardType == CardType::RedItem)
                {
                    for(auto& b : gs.EnemyBoard)
                    {
                        result.emplace_back(GameActionFactory::UseItem(c.InstanceId, b.InstanceId));
                    }
                }
                else if(c.CardType == CardType::BlueItem && c.DefenseValue != 0)
                {
                    for(auto& b : gs.EnemyBoard)
                    {
                        result.emplace_back(GameActionFactory::UseItem(c.InstanceId, b.InstanceId));
                    }
                }
                else if(c.CardType == CardType::BlueItem)
                {
                    for(auto& b : gs.EnemyBoard)
                    {
                        result.emplace_back(GameActionFactory::UseItem(c.InstanceId, GameAction::EnemyPlayerId));
                    }
                }
            }

            return result;
        }

        static double EvaluateGameState(GameState gs)
        {
            // TODO: Better evaluation function
            // An evaluation function is the hardest and most important part of an AI
            double result = 0.0;
            result += gs.MyPlayer.Health;
            result -= gs.EnemyPlayer.Health;
            result += gs.MyBoard.size();
            result += gs.PassiveCards.size();
            result += std::accumulate(gs.MyBoard.begin(), gs.MyBoard.end(), 0.0, [](double s, Card c) {return s + (double)c.AttackValue + (double)c.DefenseValue;});
            result -= std::accumulate(gs.EnemyBoard.begin(), gs.EnemyBoard.end(), 0.0, [](double s, Card c) {return s + (double)c.AttackValue + (double)c.DefenseValue;});
            return result;
        }
    };
}

struct Parse
{
    static Gambler Gambler(string input)
    {
        //Console.Error.WriteLine("!parse Gambler: " + input);
        std::vector<std::string> inputs = Utils::split(input, ' ');
        ::Gambler gambler;
        gambler.Health = stoi(inputs[0]),
        gambler.Mana = stoi(inputs[1]),
        gambler.DeckSize = stoi(inputs[2]),
        gambler.NextRuneThreshold = stoi(inputs[3]);
        return gambler;
    }

    static CardAbility Ability(string abilities)
    {
        auto nothing = CardAbility::Nothing;
        CardAbility result = nothing;
        result = result | (abilities.find('B') >= 0) ? CardAbility::Breakthrough : nothing;
        result = result | (abilities.find('C') >= 0) ? CardAbility::Charge : nothing;
        result = result | (abilities.find('D') >= 0) ? CardAbility::Drain : nothing;
        result = result | (abilities.find('G') >= 0) ? CardAbility::Guard : nothing;
        result = result | (abilities.find('L') >= 0) ? CardAbility::Lethal : nothing;
        result = result | (abilities.find('W') >= 0) ? CardAbility::Ward : nothing;
        return result;
    }

    static ::Card Card(string input)
    {
        //Console.Error.WriteLine("!parse Card: " + input);
        std::vector<std::string> inputs = Utils::split(input, ' ');
        ::Card card;
        card.CardNumber = stoi(inputs[0]);
        card.InstanceId = stoi(inputs[1]);
        card.Location = (BoardLocation)stoi(inputs[2]);
        card.CardType = (CardType)stoi(inputs[3]);
        card.Cost = stoi(inputs[4]);
        card.AttackValue = stoi(inputs[5]);
        card.DefenseValue = stoi(inputs[6]);
        card.Abilities = Parse::Ability(inputs[7]);
        card.MyHealthChange = stoi(inputs[8]);
        card.EnemyHealthChange = stoi(inputs[9]);
        card.CardDraw = stoi(inputs[10]);
        return card;
    }

    static ::GameState GameStateFromConsole()
    {
        ::GameState gs;
        string line;
        cin >> line; cin.ignore();
        gs.MyPlayer = Parse::Gambler(line);
        cin >> line; cin.ignore();
        gs.EnemyPlayer = Parse::Gambler(line);
        cin >> gs.EnemyHandCount; cin.ignore();
        cin >> gs.CardCount; cin.ignore();

        for(int i = 0; i < gs.CardCount; i++)
        {
            cin >> line; cin.ignore();
            ::Card card = Parse::Card(line);
            switch(card.Location)
            {
                case BoardLocation::EnemySide:
                    gs.EnemyBoard.emplace_back(card);
                    break;
                case BoardLocation::InHand:
                    gs.MyHand.emplace_back(card);
                    break;
                case BoardLocation::PlayerSide:
                    gs.MyBoard.emplace_back(card);
                    break;
                case BoardLocation::PlayerSidePassive:
                    gs.PassiveCards.emplace_back(card);
                    break;
            }
        }
        return gs;
    }

    static ::GameState GameState(queue<string> lines)
    {
        ::GameState gs;
        gs.MyPlayer = Parse::Gambler(lines.front()); lines.pop();
        gs.EnemyPlayer = Parse::Gambler(lines.front()); lines.pop();
        gs.EnemyHandCount = stoi(lines.front()); lines.pop();
        gs.CardCount = stoi(lines.front()); lines.pop();

        for(int i = 0; i < gs.CardCount; i++)
        {
            ::Card card = Parse::Card(lines.front()); lines.pop();

            switch(card.Location)
            {
                case BoardLocation::EnemySide:
                    gs.EnemyBoard.emplace_back(card);
                    break;
                case BoardLocation::InHand:
                    gs.MyHand.emplace_back(card);
                    break;
                case BoardLocation::PlayerSide:
                    gs.MyBoard.emplace_back(card);
                    break;
                case BoardLocation::PlayerSidePassive:
                    gs.PassiveCards.emplace_back(card);
                    break;
            }
        }
        return gs;
    }
};

/**
* Auto-generated code below aims at helping you parse
* the standard input according to the problem statement.
*
int main()
{

    // game loop
    while(1) {
        for(int i = 0; i < 2; i++) {
            int playerHealth;
            int playerMana;
            int playerDeck;
            int playerRune;
            cin >> playerHealth >> playerMana >> playerDeck >> playerRune; cin.ignore();
        }
        int opponentHand;
        cin >> opponentHand; cin.ignore();
        int cardCount;
        cin >> cardCount; cin.ignore();
        for(int i = 0; i < cardCount; i++) {
            int cardNumber;
            int instanceId;
            int location;
            int cardType;
            int cost;
            int attack;
            int defense;
            string abilities;
            int myHealthChange;
            int opponentHealthChange;
            int cardDraw;
            cin >> cardNumber >> instanceId >> location >> cardType >> cost >> attack >> defense >> abilities >> myHealthChange >> opponentHealthChange >> cardDraw; cin.ignore();
        }

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        cout << "PASS" << endl;
    }
}
*/

int mainReal()
{
    int turn = 0;

    const int draftTurnCount = 30;
    const int lastTurn = draftTurnCount + 50;
    int curve[] = { 2, 8, 7, 5, 4, 2, 2 };

    // game loop
    while(true)
    {
        GameState gs = Parse::GameStateFromConsole();
        cerr << gs.ToString() << endl;

        if(turn < lastTurn)
        {
            ++turn;
        }

        if(turn <= draftTurnCount)
        {
            cout << DraftPhase::GetBestCard(gs.MyHand, curve) << endl;
        }
        else
        {
            cout << BattlePhase::GraphSolver::ProcessTurn(gs) << endl;
        }
    }
}

#if CCGDeveloper

int mainTest()
{
    int turn = 0;

    const int draftTurnCount = 30;
    const int lastTurn = draftTurnCount + 50;
    int curve[] = { 2, 8, 7, 5, 4, 2, 2 };

    Stopwatch sw;
    // game loop
    //while (true)
    {
        sw.Restart();
        GameState gs = Parse::GameState(std::queue<string>({
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
            }));
        
        printError(gs.ToString());

        cout << BattlePhase::GraphSolver::ProcessTurn(gs) << endl;

        cerr << "Turn took " << sw.ElapsedMilliseconds() << " ms" << endl;
    }
    return 0;
}
#endif


int main()
{
#if defined(CCGDeveloper)
    mainTest();
#else
    mainReal();
#endif
}