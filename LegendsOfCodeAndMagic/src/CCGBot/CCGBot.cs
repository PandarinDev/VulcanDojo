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
            var firstPlayer = new Gambler();
            var secondPlayer = new Gambler();
            var myHand = new List<Card>();
            var myBoard = new List<Card>();
            var enemyBoard = new List<Card>();

            int turn = 0;

            const int draftTurnCount = 30;
            const int lastTurn = draftTurnCount + 50;
            int[] curve = new int[] { 2, 8, 7, 5, 4, 2, 2 };

            Stopwatch stopwatch = new Stopwatch();
            // game loop
            while (true)
            {
                stopwatch.Restart();

                firstPlayer = Parse.Gambler(Console.ReadLine());
                secondPlayer = Parse.Gambler(Console.ReadLine());
                int opponentHandCount = int.Parse(Console.ReadLine());
                int cardCount = int.Parse(Console.ReadLine());

                for (int i = 0; i < cardCount; i++)
                {
                    Card card = Parse.Card(Console.ReadLine());
                    
                    switch (card.Location)
                    {
                        case BoardLocation.EnemySide:
                            enemyBoard.Add(card);
                            break;
                        case BoardLocation.InHand:
                            myHand.Add(card);
                            break;
                        case BoardLocation.PlayerSide:
                            myBoard.Add(card);
                            break;
                    }
                }

                if (turn < lastTurn)
                {
                    ++turn;
                }
                
                if (turn <= draftTurnCount)
                {
                    Console.WriteLine(DraftPhase.GetBestCard(myHand, curve));
                }
                else
                {
                    Console.WriteLine(BattlePhase.GetBestSummon(turn, enemyBoard, myHand, myBoard) + BattlePhase.Attack(enemyBoard, myBoard));
                }
                enemyBoard.Clear();
                myHand.Clear();
                myBoard.Clear();

                Console.Error.WriteLine($"Turn took {stopwatch.ElapsedMilliseconds} ms");
            }
        }
    }

    public static class Parse
    {
        public static Gambler Gambler(string input)
        {
            string[] inputs = input.Split(' ');
            var gambler = new Gambler
            {
                PlayerHealth = int.Parse(inputs[0]),
                PlayerMana = int.Parse(inputs[1]),
                PlayerDeck = int.Parse(inputs[2]),
                PlayerRune = int.Parse(inputs[3])
            };
            return gambler;
        }

        public static Card Card(string input)
        {
            string[] inputs = input.Split(' ');
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
                OpponentHealthChange = int.Parse(inputs[9]),
                CardDraw = int.Parse(inputs[10])
            };
            return card;
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
            value -= (double)card.OpponentHealthChange / 3;
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

    public static class BattlePhase
    {
        public static string GetBestSummon(int turn, List<Card> enemyBoard, List<Card> myHand, List<Card> myBoard)
        {
            int mana = turn - 30;
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
                string target = "";
                if (card.CardType == CardType.GreenItem)
                {
                    if (myBoard.Count != 0)
                    {
                        target += myBoard[0].InstanceId;
                    }
                    else
                    {
                        target = (command.Split(' '))[1];
                    }
                }
                //TODO: improve debuff items
                if (card.CardType == CardType.RedItem || 
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

                if (card.CardType == 0)
                {
                    ++boardCount;
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
                target = "";
                mana -= card.Cost;
                myHand.Remove(card);
            }

            return command;
        }

        public static Card GetMostExpensiveCard(int mana, int myBoardCount, int enemyBoardCount, List<Card> myHand)
        {
            foreach (Card card in myHand)
            {
                //Console.Error.WriteLine(card.cost);
                int maxCost = 0;
                if (card.Cost <= mana)
                {
                    if ((card.CardType == 0 && myBoardCount < 6) || 
                        card.CardType == CardType.BlueItem || 
                        (card.CardType == CardType.GreenItem && myBoardCount != 0) || 
                        (card.CardType == CardType.RedItem && enemyBoardCount != 0))
                    {
                        if (maxCost <= card.Cost)
                        {
                            maxCost = card.Cost;
                            return card;
                        }
                    }
                }
            }
            return null;
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
                    attacks += "ATTACK " + card.InstanceId + " " + enemyTreat[0].InstanceId + ";";
                    if (card.Abilities.Contains("L"))
                    {
                        enemyTreat[0].DefenseValue = 0;
                    }
                    else
                    {
                        enemyTreat[0].DefenseValue -= card.AttackValue;
                    }
                    if (enemyTreat[0].DefenseValue <= 0)
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

    public class Gambler
    {
        public int PlayerHealth { get; set; }
        public int PlayerMana { get; set; }
        public int PlayerDeck { get; set; }
        public int PlayerRune { get; set; }
        public int Identity { get; set; }
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
        public int OpponentHealthChange { get; set; }
        public int CardDraw { get; set; }
    }

    #region enums
    public enum BoardLocation
    {
        EnemySide = -1,
        InHand = 0,
        PlayerSide = 1
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
    }

    #endregion
}
