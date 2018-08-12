using System;
using System.Linq;
using System.Collections.Generic;
using System.Diagnostics;

namespace CCG
{
    class Player
    {
        static void Main(string[] args)
        {
            int turn = 0;

            const int draftTurnCount = 30;
            const int lastTurn = draftTurnCount + 50;
            int[] curve = new int[] { 2, 8, 7, 5, 4, 2, 2 };

            Stopwatch sw = new Stopwatch();
            // game loop
            while (true)
            {
                sw.Restart();
                GameState gs = Parse.GameStateFromConsole();
                Console.Error.WriteLine($"{gs}");

                if (turn < lastTurn)
                {
                    ++turn;
                }

                //if (turn <= draftTurnCount)
                {
                    //Console.WriteLine(DraftPhase.GetBestCard(gs.MyHand, curve));
                }
               // else
                {
                    Console.WriteLine(BattlePhase.GraphSolver.ProcessTurn(gs));
                }

                Console.Error.WriteLine($"Turn took {sw.ElapsedTicks} ticks {sw.ElapsedMilliseconds} ms");
            }
        }
    }

    public static class DraftPhase
    {
        /// <summary>
        /// Represent a turn in the draft phase, 
        /// basically selects the card that we should pick
        /// </summary>
        public static string GetBestCard(List<Card> picks, int[] curve)
        {
            const int possiblePickCount = 3;
            double maxValue = -10000;
            int bestPickIndex = 0;
            for (int i = 0; i < possiblePickCount; i++)
            {
                double cardValue = GetValue(picks[i], curve);
                if (cardValue >= maxValue)
                {
                    maxValue = cardValue;
                    bestPickIndex = i;
                }
            }
            CurveAdd(picks[bestPickIndex].Cost, curve);
            return "PICK " + bestPickIndex;
        }

        public static double GetValue(Card card, int[] curve)
        {
            //TODO: improve red and blue item values
            double value = 0;
            value += Math.Abs(card.AttackValue);
            value += Math.Abs(card.DefenseValue);
            value += card.CardDraw;
            value += card.AbilitiesToString().Replace("-", "").Replace("C", "C2").Replace("W", "W2").Length * 0.5;
            value += (double)card.MyHealthChange / 3;
            value -= (double)card.EnemyHealthChange / 3;
            value -= card.Cost * 2;
            //marginal penalty
            if (card.Cost == 0 || card.AttackValue == 0)
            {
                value -= 2;
            }
            //nonboard penalty
            if (card.CardType == CardType.RedItem ||
                card.CardType == CardType.BlueItem)
            {
                value -= 1;
            }
            //balance penalty, nerf card "Decimate"
            if (card.CardNumber == 151)
            {
                value -= 94;
            }
            if (HaveEnoughInManaCurve(card.Cost, curve))
            {
                value -= 1;
            }

            return value;
        }

        public static bool HaveEnoughInManaCurve(int cost, int[] curve)
        {
            if (cost > 1 && cost < 7 && curve[cost - 1] <= 0)
            {
                return true;
            }
            if (cost <= 1 && curve[0] <= 0)
            {
                return true;
            }
            if (cost > 6 && curve[6] <= 0)
            {
                return true;
            }
            return false;
        }

        public static void CurveAdd(int cost, int[] curve)
        {
            int place = (cost - 1).Clamp(0, 6);
            curve[place] -= 1;
        }
    }

    namespace BattlePhase
    {
        public static class GraphSolver
        {
            static Stopwatch sw = new Stopwatch();

            public static string ProcessTurn(GameState gs)
            {
                sw.Restart();
                ActionSequence seq = DecideOnBestActionSequence(gs);
                return seq.ToString();
            }

            public static ActionSequence DecideOnBestActionSequence(GameState initialGameSate)
            {
                var possibleStates = new Queue<Tuple<GameState, ActionSequence>>();

                double bestValue = -100000000.0;
                ActionSequence bestSeq = new ActionSequence();
                possibleStates.Enqueue(new Tuple<GameState, ActionSequence>(initialGameSate, bestSeq));
                
                int counter = 0;
                while (possibleStates.Count > 0)
                {
                    counter++;
                    var state = possibleStates.Dequeue();
                    GameState gs = state.Item1;
                    ActionSequence toState = state.Item2;

                    if (gs.EnemyPlayer.Health <= 0)
                    {
                        Console.Error.WriteLine($"GraphSolver found winning sequence");
                        bestSeq = toState;
                        break;
                    }

                    double value = EvaluateGameState(gs);
                    if (value > bestValue)
                    {
                        bestSeq = toState;
                        bestValue = value;
                    }

                    var actions = GetPossibleActions(gs);
                    

                    var acount = actions.Count;
                    for (var i = 0; i < actions.Count; ++i)
                    {
                        var action = actions[i];
                        GameState actionGameState = Simulator.SimulateAction(gs, action);
                        possibleStates.Enqueue(new Tuple<GameState, ActionSequence>(actionGameState, toState.Extended(action)));
                    }

                    //if (counter % 200 == 0)
                    //{
                    //    Console.Error.WriteLine($"GraphSolver elapsed time: {sw.ElapsedMilliseconds} ms");
                    //}

                    if (sw.ElapsedMilliseconds > 9500)
                    {
                        Console.Error.WriteLine($"GraphSolver took to much time, breaking out");
                        break;
                    }
                }

                var elapsed = sw.ElapsedMilliseconds;
                Console.Error.WriteLine($"GraphSolver finished in {elapsed} ms with {counter} nodes");
                Console.Error.WriteLine($"GraphSolver Chosen action has value: {bestValue} , is {bestSeq}");
                return bestSeq;
            }

            public static List<GameAction> GetPossibleActions(GameState gs)
            {
                var result = new List<GameAction>();

                /* Possible actions are: 
                 * - attacking with creature to 
                 *      -player 
                 *      -other creature
                 * - play card
                 *      - summon creature
                 *      - use item
                 * 
                 * */
                 
                bool enemyHasGuard = gs.EnemyBoard.Any(c => c.HasGuard());
                if (!enemyHasGuard)
                {
                    // attack player
                    result.AddRange(gs.MyBoard.Where(c => !c.DidAttack)
                        .Select(c => GameActionFactory.CreatureAttack(c.InstanceId, GameAction.EnemyPlayerId)));

                    // attack creatures
                    result.AddRange(gs.MyBoard.Join(gs.EnemyBoard, c => !c.DidAttack, _ => true, (c, e) => new { Card = c, Enemy = e })
                        .Select(p => GameActionFactory.CreatureAttack(p.Card.InstanceId, p.Enemy.InstanceId)));
                }
                else
                {
                    // attack guards
                    result.AddRange(gs.MyBoard.Join(gs.EnemyBoard, c => !c.DidAttack, e => e.HasGuard(), 
                        (c, e) => new { Card = c.InstanceId, Enemy = e.InstanceId })
                        .Select(p => GameActionFactory.CreatureAttack(p.Card, p.Enemy)));
                }

                var creaturesInHand = gs.MyHand.FindAll(c => c.CardType == CardType.Creature && c.Cost <= gs.MyPlayer.Mana);
                var itemsInHand = gs.MyHand.FindAll(c => c.CardType != CardType.Creature && c.Cost <= gs.MyPlayer.Mana);

                result.AddRange(itemsInHand.FindAll(i => i.CardType == CardType.GreenItem)
                    .Join(gs.MyBoard, _ => true, _=> true, (i, t) => new { Item=i.InstanceId, Target=t.InstanceId})
                    .Select(p=> GameActionFactory.UseItem(p.Item, p.Target)));

                result.AddRange(itemsInHand.FindAll(i => i.CardType == CardType.RedItem)
                    .Join(gs.EnemyBoard, _ => true, _ => true, (i, t) => new { Item = i.InstanceId, Target = t.InstanceId })
                    .Select(p => GameActionFactory.UseItem(p.Item, p.Target)));

                result.AddRange(itemsInHand.FindAll(i => i.CardType == CardType.BlueItem && i.DefenseValue != 0)
                    .Join(gs.EnemyBoard, _ => true, _ => true, (i, t) => new { Item = i.InstanceId, Target = t.InstanceId })
                    .Select(p => GameActionFactory.UseItem(p.Item, p.Target)));

                result.AddRange(itemsInHand.FindAll(i => i.CardType == CardType.BlueItem)
                    .Select(p => GameActionFactory.UseItem(p.InstanceId, GameAction.EnemyPlayerId)));

                result.AddRange(creaturesInHand.Select(c => GameActionFactory.SummonCreature(c.InstanceId)));

                return result;
            }

            public static double EvaluateGameState(GameState gs)
            {
                // TODO: Better evaluation function
                // An evaluation function is the hardest and most important part of an AI
                double result = 0.0;
                result += gs.MyPlayer.Health;
                result -= gs.EnemyPlayer.Health;
                result += gs.MyBoard.Count;
                result += gs.PassiveCards.Count;
                result += gs.MyBoard.Sum(c => c.AttackValue+c.DefenseValue);
                result -= gs.EnemyBoard.Sum(c => c.AttackValue + c.DefenseValue);
                return result;
            }
        }

        /// <summary>
        /// Processes a turn of the battle phase based on rules written by the programmer.
        /// Good for a baseline against AIs.
        /// </summary>
        public static class HumanSolver
        {
            public static string ProcessTurn(GameState gs)
            {
                return GetBestSummon(gs.MyPlayer.Mana, gs.EnemyBoard, gs.MyHand, gs.MyBoard) + Attack(gs.EnemyBoard, gs.MyBoard);
            }

            public static string GetBestSummon(int mana, List<Card> enemyBoard, List<Card> myHand, List<Card> myBoard)
            {
                int boardCount = myBoard.Count;
                int enemyBoardCount = enemyBoard.Count;
                var playList = new List<int>();
                var card = new Card();
                string command = "";
                while (true)
                {
                    card = GetMostExpensiveCard(mana, boardCount, enemyBoardCount, myHand);
                    if (card == null)
                    {
                        break;
                    }

                    int target = 0;
                    if (card.CardType == CardType.GreenItem)
                    {
                        if (myBoard.Count != 0)
                        {
                            target += myBoard[0].InstanceId;
                        }
                        else
                        {
                            Console.Error.WriteLine("Error: Tried to cast green item when I dont have a minion");
                        }
                    }
                    else if (card.CardType == CardType.RedItem ||
                        (card.CardType == CardType.BlueItem && card.DefenseValue < 0))
                    {
                        if (enemyBoard.Count == 0)
                        {
                            target += -1;
                        }
                        else
                        {
                            target += enemyBoard[0].InstanceId;
                            if (enemyBoard[0].DefenseValue <= -card.DefenseValue)
                            {
                                enemyBoard.RemoveAt(0);
                            }
                        }
                    }

                    if (card.CardType == CardType.Creature)
                    {
                        boardCount += 1;
                        if (card.HasCharge())
                        {
                            myBoard.Add(card);
                        }
                        command += "SUMMON " + card.InstanceId + ";";
                    }
                    else
                    {
                        command += "USE " + card.InstanceId + " " + target + ";";
                    }

                    target = 0;
                    mana -= card.Cost;
                    myHand.Remove(card);
                }

                return command;
            }

            public static Card GetMostExpensiveCard(int mana, int myBoardCount, int enemyBoardCount, List<Card> myHand)
            {
                Card cardToPlay = null;
                int maxCost = 0;
                foreach (Card card in myHand)
                {
                    if (card.Cost <= mana)
                    {
                        if ((card.CardType == CardType.Creature && myBoardCount < 6) ||
                            card.CardType == CardType.BlueItem ||
                            (card.CardType == CardType.GreenItem && myBoardCount > 0) ||
                            (card.CardType == CardType.RedItem && enemyBoardCount > 0))
                        {
                            if (maxCost <= card.Cost)
                            {
                                Console.Error.WriteLine($"Considering to play {card.InstanceId}");
                                maxCost = card.Cost;
                                cardToPlay = card;
                            }
                        }
                    }
                }
                return cardToPlay;
            }

            public static string Attack(List<Card> enemyBoard, List<Card> myBoard)
            {
                string attacks = "";
                var enemyTreat = new List<Card>();
                foreach (Card card in enemyBoard)
                {
                    if (card.HasGuard())
                    {
                        enemyTreat.Add(card);
                    }
                }
                foreach (Card card in myBoard)
                {
                    if (enemyTreat.Count != 0)
                    {
                        Card enemyCard = enemyTreat[0];
                        attacks += "ATTACK " + card.InstanceId + " " + enemyCard.InstanceId + ";";
                        if (enemyCard.HasWard())
                        {
                            enemyCard.RemoveWard();
                        }
                        else if (card.HasLethal())
                        {
                            enemyCard.DefenseValue = 0;
                        }
                        else
                        {
                            enemyCard.DefenseValue -= card.AttackValue;
                        }
                        if (enemyCard.DefenseValue <= 0)
                        {
                            enemyTreat.RemoveAt(0);
                        }
                    }
                    else
                    {
                        attacks += "ATTACK " + card.InstanceId + " -1;";
                    }
                }

                return attacks;
            }
        }
    }

    public static class Simulator
    {
        public static GameState SimulateAction(GameState gs, GameAction a)
        {
            GameState state = gs.Copy();
            switch (a.Type)
            {
                case ActionType.CreatureAttack:
                    AttackAction(ref state, a.Id, a.TargetId);
                    break;
                case ActionType.SummonCreature:
                    SummonCreatureAction(ref state, a.Id);
                    break;
                case ActionType.UseItem:
                    UseItemAction(ref state, a.Id, a.TargetId);
                    break;
                default:
                    break;
            }
            return state;
        }

        private static void AttackAction(ref GameState state, int attackerId, int defenderId)
        {
            var attacker = state.MyBoard.Find(c => c.InstanceId == attackerId);

            if (defenderId != GameAction.EnemyPlayerId)
            {
                var defender = state.EnemyBoard.Find(c => c.InstanceId == defenderId);
                int attackerHpBefore = attacker.DefenseValue;
                int defenderHpBefore = defender.DefenseValue;
                AttackCreature(ref attacker, ref defender);
                if (attacker.DefenseValue <= 0)
                {
                    state.MyBoard.Remove(attacker);
                    state.CardCount -= 1;
                }
                if (defender.DefenseValue <= 0)
                {
                    state.EnemyBoard.Remove(defender);
                    state.CardCount -= 1;
                    if (attacker.HasBreakthrough())
                    {
                        state.EnemyPlayer.Health += defender.DefenseValue;
                    }
                }
                Drain(ref state.MyPlayer, attacker, Math.Max(0, defenderHpBefore - Math.Max(0, defender.DefenseValue)));
                Drain(ref state.EnemyPlayer, defender, attackerHpBefore - Math.Max(0, attacker.DefenseValue));
            }
            else
            {
                state.EnemyPlayer.Health -= attacker.AttackValue;
                Drain(ref state.MyPlayer, attacker, attacker.AttackValue);
                attacker.DidAttack = true;
            }
        }

        private static void Drain(ref Gambler healedPlayer, Card attacker, int amount)
        {
            if (attacker.HasDrain())
            {
                healedPlayer.Health += Math.Max(0, amount);
            }
            //healedPlayer.Health += ((int)(attacker.Abilities & Card.Ability.Drain) >> 2) * Math.Max(0, amount);
        }

        private static void UseItemAction(ref GameState state, int itemId, int targetId)
        {
            var item = state.MyHand.Find(c => c.InstanceId == itemId);
            state.MyPlayer.Mana -= item.Cost;
            state.MyHand.Remove(item);
            state.CardCount -= 1;

            state.MyPlayer.Health += item.MyHealthChange;
            state.EnemyPlayer.Health += item.EnemyHealthChange;

            if (item.CardType == CardType.GreenItem)
            {
                var creature = state.MyBoard.Find(c => c.InstanceId == targetId);
                creature.AttackValue += item.AttackValue;
                creature.DefenseValue += item.DefenseValue;
                creature.AddAbility(item.Abilities);
            }
            else if(item.CardType == CardType.RedItem || 
                (item.CardType == CardType.BlueItem && targetId != GameAction.EnemyPlayerId))
            {
                var creature = state.EnemyBoard.Find(c => c.InstanceId == targetId);
                creature.AttackValue += item.AttackValue;
                creature.DefenseValue += item.DefenseValue;
                creature.RemoveAbilty(item.Abilities);
            }
        }

        /// <summary>
        /// Simulates an attack action between two creatures.
        /// The given cards will be modified, ie their health will change / lose ward
        /// after this method returns.
        /// </summary>
        /// <param name="attacker"></param>
        /// <param name="defender"></param>
        public static void AttackCreature(ref Card attacker, ref Card defender)
        {
            HalfAttack(ref attacker, ref defender);

            if (!defender.DidAttack)
            {
                HalfAttack(ref defender, ref attacker);
            }
        }

        private static void HalfAttack(ref Card attacker, ref Card defender)
        {
            if (attacker.AttackValue > 0)
            {
                if (defender.HasWard())
                    defender.RemoveWard();
                else if (attacker.HasLethal())
                {
                    defender.DefenseValue = 0;
                }
                else
                    defender.DefenseValue -= attacker.AttackValue;
            }
            attacker.DidAttack = true;
        }

        private static void SummonCreatureAction(ref GameState state, int creatureId)
        {
            var toSummon = state.MyHand.Find(c => c.InstanceId == creatureId);
            state.MyPlayer.Mana -= toSummon.Cost;
            state.MyHand.Remove(toSummon);

            if (toSummon.HasCharge())
            {
                state.MyBoard.Add(toSummon);
                toSummon.Location = BoardLocation.PlayerSide;
            }
            else
            {
                state.PassiveCards.Add(toSummon);
                toSummon.Location = BoardLocation.PlayerSidePassive;
            }
        }
    }

    public static class Parse
    {
        public static GameState GameStateFromConsole()
        {
            GameState gs = new GameState
            {
                MyPlayer = Parse.Gambler(Console.ReadLine()),
                EnemyPlayer = Parse.Gambler(Console.ReadLine()),
                EnemyHandCount = int.Parse(Console.ReadLine()),
                CardCount = int.Parse(Console.ReadLine())
            };
            
            for (int i = 0; i < gs.CardCount; i++)
            {
                Card card = Parse.Card(Console.ReadLine());
                switch (card.Location)
                {
                    case BoardLocation.EnemySide:
                        gs.EnemyBoard.Add(card);
                        break;
                    case BoardLocation.InHand:
                        gs.MyHand.Add(card);
                        break;
                    case BoardLocation.PlayerSide:
                        gs.MyBoard.Add(card);
                        break;
                    case BoardLocation.PlayerSidePassive:
                        gs.PassiveCards.Add(card);
                        break;
                }
            }
            return gs;
        }

        public static GameState GameState(Queue<string> lines)
        {
            GameState gs = new GameState
            {
                MyPlayer = Parse.Gambler(lines.Dequeue()),
                EnemyPlayer = Parse.Gambler(lines.Dequeue()),
                EnemyHandCount = int.Parse(lines.Dequeue()),
                CardCount = int.Parse(lines.Dequeue())
            };

            for (int i = 0; i < gs.CardCount; i++)
            {
                Card card = Parse.Card(lines.Dequeue());

                switch (card.Location)
                {
                    case BoardLocation.EnemySide:
                        gs.EnemyBoard.Add(card);
                        break;
                    case BoardLocation.InHand:
                        gs.MyHand.Add(card);
                        break;
                    case BoardLocation.PlayerSide:
                        gs.MyBoard.Add(card);
                        break;
                    case BoardLocation.PlayerSidePassive:
                        gs.PassiveCards.Add(card);
                        break;
                }
            }
            return gs;
        }

        public static Gambler Gambler(string input)
        {
            //Console.Error.WriteLine("!parse Gambler: " + input);
            string[] inputs = input.Split(' ');
            var gambler = new Gambler
            {
                Health = int.Parse(inputs[0]),
                Mana = int.Parse(inputs[1]),
                DeckSize = int.Parse(inputs[2]),
                NextRuneThreshold = int.Parse(inputs[3])
            };
            return gambler;
        }

        public static Card Card(string input)
        {
            //Console.Error.WriteLine("!parse Card: " + input);
            string[] inputs = input.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
            var card = new Card
            {
                CardNumber = int.Parse(inputs[0]),
                InstanceId = int.Parse(inputs[1]),
                Location = (BoardLocation)int.Parse(inputs[2]),
                CardType = (CardType)int.Parse(inputs[3]),
                Cost = int.Parse(inputs[4]),
                AttackValue = int.Parse(inputs[5]),
                DefenseValue = int.Parse(inputs[6]),
                Abilities = Parse.Ability(inputs[7]),
                MyHealthChange = int.Parse(inputs[8]),
                EnemyHealthChange = int.Parse(inputs[9]),
                CardDraw = int.Parse(inputs[10])
            };
            return card;
        }

        public static Card.Ability Ability(string abilities)
        {
            var nothing = CCG.Card.Ability.Nothing;
            Card.Ability result = nothing;
            result |= abilities.Contains("B") ? CCG.Card.Ability.Breakthrough : nothing;
            result |= abilities.Contains("C") ? CCG.Card.Ability.Charge : nothing;
            result |= abilities.Contains("D") ? CCG.Card.Ability.Drain : nothing;
            result |= abilities.Contains("G") ? CCG.Card.Ability.Guard : nothing;
            result |= abilities.Contains("L") ? CCG.Card.Ability.Lethal : nothing;
            result |= abilities.Contains("W") ? CCG.Card.Ability.Ward : nothing;

            return result;
        }
    }


    public class ActionSequence : ICloneable
    {
        public List<GameAction> Actions = new List<GameAction>();

        public override string ToString()
        {
            string actions = string.Join(";", Actions.Select(a => a.ToString()));
            return string.IsNullOrEmpty(actions) ? "PASS" : actions;
        }

        public ActionSequence Extended(GameAction a)
        {
            var copy = this.Clone();
            copy.Add(a);
            return copy;
        }

        public void Add(GameAction a)
        {
            Actions.Add(a);
        }

        object ICloneable.Clone()
        {
            return this.Clone();
        }

        /// <summary>
        /// Gives back a shallow copy of actions
        /// </summary>
        public ActionSequence Clone()
        {
            ActionSequence c = new ActionSequence
            {
                Actions = new List<GameAction>(Actions)
            };
            return c;
        }
    }

    public enum ActionType
    {
        CreatureAttack,
        SummonCreature,
        UseItem
    }

    public class GameAction
    {
        public const int EnemyPlayerId = -1;

        public ActionType Type { get; }
        public int Id { get; }
        public int TargetId { get; } = EnemyPlayerId;

        public GameAction(ActionType type)
        {
            Type = type;
        }

        public GameAction(ActionType type, int iid) : this(type)
        {
            Id = iid;
        }

        public GameAction(ActionType type, int iid, int targetId) : this(type, iid)
        {
            TargetId = targetId;
        }

        public override string ToString()
        {
            switch (Type)
            {
                case ActionType.CreatureAttack:
                    return $"ATTACK {Id} {TargetId}";
                case ActionType.SummonCreature:
                    return $"SUMMON {Id}";
                case ActionType.UseItem:
                    return $"USE {Id} {TargetId}";
                default:
                    return "PASS";
            }
        }
    }

    public static class GameActionFactory
    {
        public static GameAction SummonCreature(int iid)
        {
            return new GameAction(ActionType.SummonCreature, iid);
        }

        public static GameAction CreatureAttack(int iid, int targetId)
        {
            return new GameAction(ActionType.CreatureAttack, iid, targetId);
        }

        public static GameAction UseItem(int iid, int targetId)
        {
            return new GameAction(ActionType.UseItem, iid, targetId);
        }
    }

    public class GameState
    {
        public Gambler MyPlayer;
        public Gambler EnemyPlayer;
        public int EnemyHandCount;
        public int CardCount; // Cards in my hand + on the board
        public List<Card> AllCards = new List<Card>();
        public List<Card> MyHand = new List<Card>();
        public List<Card> MyBoard = new List<Card>();
        public List<Card> EnemyBoard = new List<Card>();
        public List<Card> PassiveCards = new List<Card>();

        public GameState Copy()
        {
            GameState gs = new GameState()
            {
                MyPlayer = MyPlayer.Copy(),
                EnemyPlayer = EnemyPlayer.Copy(),
                EnemyHandCount = EnemyHandCount,
                CardCount = CardCount,
                MyHand = MyHand.Copy(),
                MyBoard = MyBoard.Copy(),
                EnemyBoard = EnemyBoard.Copy(),
                PassiveCards = PassiveCards.Copy()
            };
            return gs;
        }

        public override string ToString()
        {
            Func<IEnumerable<Card>, string> listToString = 
                ((IEnumerable<Card> a) => string.Join("\n", a.Select(c => c.ToString()).ToArray()) );
            string cards = string.Join("\n", 
                new string[] { listToString(MyHand), listToString(MyBoard), listToString(EnemyBoard), listToString(PassiveCards) }
                .Where(s => !string.IsNullOrEmpty(s)));
            return $"{MyPlayer}\n{EnemyPlayer}\n{EnemyHandCount}\n{CardCount}\n{cards}";
        }

        public override bool Equals(object o)
        {
            var c = o as GameState;
            if (c != null)
            {
                bool result = MyPlayer != null && MyPlayer.Equals(c.MyPlayer) &&
                EnemyPlayer != null && EnemyPlayer.Equals(c.EnemyPlayer) &&
                EnemyHandCount == c.EnemyHandCount &&
                CardCount == c.CardCount &&
                MyHand.SequenceEqual(c.MyHand) &&
                MyBoard.SequenceEqual(c.MyBoard) &&
                EnemyBoard.SequenceEqual(c.EnemyBoard) &&
                PassiveCards.SequenceEqual(c.PassiveCards);
                return result;
            }
            return false;
        }
    }

    public class Gambler
    {
        public int Health { get; set; }
        public int Mana { get; set; }
        public int DeckSize { get; set; }
        public int NextRuneThreshold { get; set; }

        public Gambler Copy()
        {
            return (Gambler)this.MemberwiseClone();
        }

        public override string ToString()
        {
            return $"{Health} {Mana} {DeckSize} {NextRuneThreshold}";
        }

        public override bool Equals(object o)
        {
            var c = o as Gambler;
            if (c != null)
            {
                bool result = Health == c.Health && Mana == c.Mana &&
                DeckSize == c.DeckSize && NextRuneThreshold == c.NextRuneThreshold;
                return result;
            }
            return false;
        }
    }

    public class Card
    {
        public enum Ability
        {
            Nothing = 0x00,
            Breakthrough = 0x01,
            Charge = 0x02,
            Drain = 0x04,
            Guard = 0x08,
            Lethal = 0x10,
            Ward = 0x20,
            All = 0x3F,
        }

        public int CardNumber { get; set; }
        public int InstanceId { get; set; }
        public BoardLocation Location { get; set; }
        public CardType CardType { get; set; }
        public int Cost { get; set; }
        public int AttackValue { get; set; }
        public int DefenseValue { get; set; }
        public Ability Abilities { get; set; }
        public int MyHealthChange { get; set; }
        public int EnemyHealthChange { get; set; }
        public int CardDraw { get; set; }

        public bool DidAttack { get; set; } = false;

        public bool HasBreakthrough() => HasAbility(Ability.Breakthrough);
        public bool HasCharge() => HasAbility(Ability.Charge);
        public bool HasDrain() => HasAbility(Ability.Drain);
        public bool HasGuard() => HasAbility(Ability.Guard);
        public bool HasLethal() => HasAbility(Ability.Lethal);
        public bool HasWard() => HasAbility(Ability.Ward);
        public bool HasAbility(Ability a) => (Abilities & a) != Ability.Nothing;

        public void AddBreakthrough() => AddAbility(Ability.Breakthrough);
        public void AddCharge() => AddAbility(Ability.Charge);
        public void AddDrain() => AddAbility(Ability.Drain);
        public void AddGuard() => AddAbility(Ability.Guard);
        public void AddLethal() => AddAbility(Ability.Lethal);
        public void AddWard() => AddAbility(Ability.Ward);
        public void AddAbility(Ability a) => Abilities = (Abilities | a);

        public void RemoveBreakthrough() => RemoveAbilty(Ability.Breakthrough);
        public void RemoveCharge() => RemoveAbilty(Ability.Charge);
        public void RemoveDrain() => RemoveAbilty(Ability.Drain);
        public void RemoveGuard() => RemoveAbilty(Ability.Guard);
        public void RemoveLethal() => RemoveAbilty(Ability.Lethal);
        public void RemoveWard() => RemoveAbilty(Ability.Ward);
        public void RemoveAbilty(Ability a) => Abilities = (Abilities & ~a) & Ability.All;

        public Card Copy()
        {
            return (Card)this.MemberwiseClone();
        }

        public override string ToString()
        {
            return $"{CardNumber} {InstanceId} {Location} {CardType} {Cost} {AttackValue} {DefenseValue} {AbilitiesToString()} {MyHealthChange} {EnemyHealthChange} {CardDraw}";
        }
        
        public string AbilitiesToString()
        {
            string abilities = HasBreakthrough() ? "B" : "-";
            abilities += HasCharge() ? "C" : "-";
            abilities += HasDrain() ? "D" : "-";
            abilities += HasGuard() ? "G" : "-";
            abilities += HasLethal() ? "L" : "-";
            abilities += HasWard() ? "W" : "-";
            return abilities;
        }

        public override bool Equals(object o)
        {
            var c = o as Card;
            if (c != null)
            {
                bool result = CardNumber == c.CardNumber && InstanceId == c.InstanceId &&
                Location == c.Location && CardType == c.CardType &&
                Cost == c.Cost && AttackValue == c.AttackValue &&
                DefenseValue == c.DefenseValue && Abilities == c.Abilities &&
                MyHealthChange == c.MyHealthChange && EnemyHealthChange == c.EnemyHealthChange &&
                CardDraw == c.CardDraw;
                return result;
            }
            return false;
        }
    }

    #region enums
    public enum BoardLocation
    {
        EnemySide = -1,
        InHand = 0,
        PlayerSide = 1,
        PlayerSidePassive = 2
    }

    public enum CardType
    {
        Creature = 0,
        GreenItem = 1,
        RedItem = 2,
        BlueItem = 3
    }
    #endregion

    #region utilities

    static class Extensions
    {
        public static T Clamp<T>(this T val, T min, T max) where T : IComparable<T>
        {
            if (val.CompareTo(min) < 0) return min;
            else if (val.CompareTo(max) > 0) return max;
            else return val;
        }

        public static List<Card> Copy(this List<Card> val)
        {
            var result = new List<Card>(val.Count);
            for (int i = 0; i < val.Count; i++)
            {
                result.Add(val[i].Copy());
            }
            return result;
        }
    }

    #endregion
}
