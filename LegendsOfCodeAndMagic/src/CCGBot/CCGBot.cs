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

            Stopwatch stopwatch = new Stopwatch();
            // game loop
            while (true)
            {
                stopwatch.Restart();
                GameState gs = Parse.GameStateFromConsole();

                if (turn < lastTurn)
                {
                    ++turn;
                }

                if (turn <= draftTurnCount)
                {
                    Console.WriteLine(DraftPhase.GetBestCard(gs.MyHand, curve));
                }
                else
                {
                    Console.WriteLine(BattlePhase.HumanSolver.ProcessTurn(gs));
                }

                Console.Error.WriteLine($"Turn took {stopwatch.ElapsedMilliseconds} ms");
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
                //Console.Error.WriteLine(cardList[i].cardNumber + " " + cardValue);
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
            value += card.Abilities.Replace("-", "").Replace("C", "C2").Replace("W", "W2").Length * 0.5;
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

            public static string ProcessTurn(GameState gs)
            {
                ActionSequence seq = DecideOnBestActionSequence(gs);
                return seq.ToString();
            }

            public static ActionSequence DecideOnBestActionSequence(GameState initialGameSate)
            {
                var possibleStates = new Queue<Tuple<GameState, ActionSequence>>();

                double bestValue = 0.0;
                ActionSequence bestSeq = new ActionSequence();
                possibleStates.Enqueue(new Tuple<GameState, ActionSequence>(initialGameSate, bestSeq));

                // TODO: Avoid infinite loop with NoAction
                while (possibleStates.Count > 0)
                {
                    var state = possibleStates.Dequeue();
                    GameState gs = state.Item1;
                    ActionSequence toState = state.Item2;

                    // could put this to separate step
                    double value = EvaluateGameState(gs);
                    if(value > bestValue)
                    {
                        bestSeq = toState;
                    }

                    var actions = GetPossibleActions(gs);
                    foreach (var action in actions)
                    {
                        GameState actionGameState = SimulateAction(gs, action);
                        possibleStates.Enqueue(new Tuple<GameState, ActionSequence>(actionGameState, toState.Extended(action)));
                    }
                }

                return bestSeq;
            }

            public static List<GameAction> GetPossibleActions(GameState gs)
            {
                var result = new List<GameAction>()
                {
                    new GameAction(ActionType.NoAction) // Doing nothing is always a valid action
                };

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
                    result.AddRange(gs.MyBoard.Select(c => GameActionFactory.CreatureAttack(c.InstanceId, GameAction.EnemyPlayerId)));

                    result.AddRange(gs.MyBoard.Join(gs.EnemyBoard, _ => true, _ => true, (c, e) => new { Card = c, Enemy = e })
                        .Select(p => GameActionFactory.CreatureAttack(p.Card.InstanceId, p.Enemy.InstanceId)));
                }
                else
                {
                    result.AddRange(gs.MyBoard.Join(gs.EnemyBoard, _ => true, e => e.HasGuard(), 
                        (c, e) => new { Card = c.InstanceId, Enemy = e.InstanceId })
                        .Select(p => GameActionFactory.CreatureAttack(p.Card, p.Enemy)));
                }

                var creaturesInHand = gs.MyHand.FindAll(c => c.CardType == CardType.Creature && c.Cost < gs.MyPlayer.Mana);
                var itemsInHand = gs.MyHand.FindAll(c => c.CardType != CardType.Creature && c.Cost < gs.MyPlayer.Mana);

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

            public static GameState SimulateAction(GameState gs, GameAction action)
            {
                return Simulator.SimulateAction(gs, action);
            }

            public static double EvaluateGameState(GameState gs)
            {
                return 0.0;
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
                        if (card.Abilities.Contains("C"))
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
                    if (card.Abilities.Contains("G"))
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
                        if (enemyCard.Abilities.Contains("W"))
                        {
                            enemyCard.Abilities = enemyCard.Abilities.Replace("W", "");
                        }
                        else if (card.Abilities.Contains("L"))
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
                case ActionType.NoAction:
                    break;
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
            }
        }

        private static void Drain(ref Gambler healedPlayer, Card attacker, int amount)
        {
            if (attacker.HasDrain())
            {
                healedPlayer.Health += Math.Max(0, amount);
            }
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

                if (item.HasBreakthrough())
                {
                    creature.AddBreakthrough();
                }
                if (item.HasCharge())
                {
                    creature.AddCharge();
                }
                if (item.HasDrain())
                {
                    creature.AddDrain();
                }
                if (item.HasGuard())
                {
                    creature.AddGuard();
                }
                if (item.HasLethal())
                {
                    creature.AddLethal();
                }
                if (item.HasWard())
                {
                    creature.AddWard();
                }
            }
            else if(item.CardType == CardType.RedItem)
            {
                var creature = state.EnemyBoard.Find(c => c.InstanceId == targetId);
                creature.AttackValue += item.AttackValue;
                creature.DefenseValue += item.DefenseValue;

                if (item.HasBreakthrough())
                {
                    creature.RemoveBreakthrough();
                }
                if (item.HasCharge())
                {
                    creature.RemoveCharge();
                }
                if (item.HasDrain())
                {
                    creature.RemoveDrain();
                }
                if (item.HasGuard())
                {
                    creature.RemoveGuard();
                }
                if (item.HasLethal())
                {
                    creature.RemoveLethal();
                }
                if (item.HasWard())
                {
                    creature.RemoveWard();
                }
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
            Queue<string> lines = new Queue<string>();
            lines.Enqueue(Console.ReadLine());
            lines.Enqueue(Console.ReadLine());
            lines.Enqueue(Console.ReadLine());
            string countString = Console.ReadLine();
            lines.Enqueue(countString);

            int count = int.Parse(countString);
            for (int i = 0; i < count; i++)
            {
                lines.Enqueue(Console.ReadLine());
            }
            return Parse.GameState(lines);
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
            Console.Error.WriteLine("!parse Gambler: " + input);
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
            Console.Error.WriteLine("!parse Card: " + input);
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
                Abilities = inputs[7],
                MyHealthChange = int.Parse(inputs[8]),
                EnemyHealthChange = int.Parse(inputs[9]),
                CardDraw = int.Parse(inputs[10])
            };
            return card;
        }
    }


    public class ActionSequence : ICloneable
    {
        public List<GameAction> Actions = new List<GameAction>();

        public override string ToString()
        {
            return "Not implemetned";
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
        NoAction,
        CreatureAttack,
        SummonCreature,
        UseItem
    }

    public class GameAction
    {
        public const int EnemyPlayerId = -1;

        public ActionType Type { get; }
        public int Id { get; }
        public int TargetId { get; }

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
        public int CardNumber { get; set; }
        public int InstanceId { get; set; }
        public BoardLocation Location { get; set; }
        public CardType CardType { get; set; }
        public int Cost { get; set; }
        public int AttackValue { get; set; }
        public int DefenseValue { get; set; }
        public string Abilities { get; set; }
        public int MyHealthChange { get; set; }
        public int EnemyHealthChange { get; set; }
        public int CardDraw { get; set; }

        public bool DidAttack { get; set; } = false;

        public bool HasBreakthrough() => Abilities.Contains("B");
        public bool HasCharge() => Abilities.Contains("C");
        public bool HasDrain() => Abilities.Contains("D");
        public bool HasGuard() => Abilities.Contains("G");
        public bool HasLethal() => Abilities.Contains("L");
        public bool HasWard() => Abilities.Contains("W");

        public void AddBreakthrough() => Abilities = 'B' + Abilities.Substring(1);
        public void AddCharge() => Abilities = Abilities.Substring(0,1) + 'C' + Abilities.Substring(2);
        public void AddDrain() => Abilities = Abilities.Substring(0,2) + 'D' + Abilities.Substring(3);
        public void AddGuard() => Abilities = Abilities.Substring(0,3) + 'G' + Abilities.Substring(4);
        public void AddLethal() => Abilities = Abilities.Substring(0,4) + 'L' + Abilities.Substring(5);
        public void AddWard() => Abilities = Abilities.Substring(0,5) + 'W';


        public void RemoveBreakthrough() => RemoveAbilty("B");
        public void RemoveCharge() => RemoveAbilty("C");
        public void RemoveDrain() => RemoveAbilty("D");
        public void RemoveGuard() => RemoveAbilty("G");
        public void RemoveLethal() => RemoveAbilty("L");
        public void RemoveWard() => RemoveAbilty("W");
        private void RemoveAbilty(string sign) => Abilities = Abilities.Replace(sign, "-");

        public Card Copy()
        {
            return (Card)this.MemberwiseClone();
        }

        public override string ToString()
        {
            return $"{CardNumber} {InstanceId} {Location} {CardType} {Cost} {AttackValue} {DefenseValue} {Abilities} {MyHealthChange} {EnemyHealthChange} {CardDraw}";
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
            return val.Select(c => c.Copy()).ToList();
        }
    }

    #endregion
}
