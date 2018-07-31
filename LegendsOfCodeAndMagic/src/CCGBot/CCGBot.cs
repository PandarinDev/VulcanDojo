using System;
using System.Linq;
using System.Collections.Generic;

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
            int[] curve = new int[] { 2, 8, 7, 5, 4, 2, 2 };

            // game loop
            while (true)
            {
                firstPlayer = Util.ParseGambler(Console.ReadLine());
                secondPlayer = Util.ParseGambler(Console.ReadLine());
                int opponentHand = int.Parse(Console.ReadLine());
                int cardCount = int.Parse(Console.ReadLine());
                for (int i = 0; i < cardCount; i++)
                {
                    Card card = Util.ParseCard(Console.ReadLine());

                    Console.Error.WriteLine(card.Location);
                    //cards.Add(card);
                    switch (card.Location)
                    {
                        case -1:
                            enemyBoard.Add(card);
                            break;
                        case 0:
                            myHand.Add(card);
                            break;
                        case 1:
                            myBoard.Add(card);
                            break;
                    }

                }

                if (turn < 42)
                {
                    ++turn;
                }
                //Console.WriteLine("PASS");
                //Console.Error.WriteLine("Debug");
                if (turn <= 30)
                {
                    Console.WriteLine(Util.GetBestCard(myHand, curve));
                }
                else
                {
                    Console.WriteLine(Util.GetBestSummon(turn, enemyBoard, myHand, myBoard) + Util.Attack(enemyBoard, myBoard));
                }
                enemyBoard.Clear();
                myHand.Clear();
                myBoard.Clear();
            }
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
        public int Location { get; set; }
        public int CardType { get; set; }
        public int Cost { get; set; }
        public int Attack { get; set; }
        public int Defense { get; set; }
        public string Abilities { get; set; }
        public int MyHealthChange { get; set; }
        public int OpponentHealthChange { get; set; }
        public int CardDraw { get; set; }
    }

    public static class Util
    {
        public static Gambler ParseGambler(string input)
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

        public static Card ParseCard(string input)
        {
            string[] inputs = input.Split(' ');
            var card = new Card
            {
                CardNumber = int.Parse(inputs[0]),
                InstanceId = int.Parse(inputs[1]),
                Location = int.Parse(inputs[2]),
                CardType = int.Parse(inputs[3]),
                Cost = int.Parse(inputs[4]),
                Attack = int.Parse(inputs[5]),
                Defense = int.Parse(inputs[6]),
                Abilities = inputs[7],
                MyHealthChange = int.Parse(inputs[8]),
                OpponentHealthChange = int.Parse(inputs[9]),
                CardDraw = int.Parse(inputs[10])
            };
            return card;
        }

        public static double GetValue(Card card, int[] curve)
        {
            //TODO: improve red and blue item values
            double value = 0;
            value += Math.Abs(card.Attack);
            value += Math.Abs(card.Defense);
            value += card.CardDraw;
            value += card.Abilities.Replace("-", "").Replace("L", "L2").Replace("W", "W2").Length * 0.5;
            value += card.MyHealthChange / 3;
            value -= card.OpponentHealthChange / 3;
            value -= card.Cost * 2;
            //marginal penalty
            if (card.Cost == 0 || card.Attack == 0)
            {
                value -= 2;
            }
            //nonboard penalty
            if (card.CardType == 2 || card.CardType == 3)
            {
                value -= 1;
            }
            //balance penalty, nerf decimate
            if (card.CardNumber == 155)
            {
                value -= 94;
            }
            if (GetCurveFit(card.Cost, curve))
            {
                --value;
            }
            if (card.Attack - card.Defense >= 2)
            {
                value += 0.01;
            }
            if (-card.Attack + card.Defense >= 2)
            {
                value -= 0.01;
            }

            return value;
        }

        public static void CurveAdd(int cost, int[] curve)
        {
            if (cost > 1 && cost < 7)
            {
                --curve[cost - 1];

            }
            if (cost <= 1)
            {
                --curve[0];
            }
            if (cost > 6)
            {
                --curve[6];
            }

        }

        public static bool GetCurveFit(int cost, int[] curve)
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

        public static string GetBestCard(List<Card> myHand, int[] curve)
        {
            double maxValue = -10000;
            int max = 0;
            for (int i = 0; i < 3; i++)
            {
                double cardValue = GetValue(myHand[i], curve);
                if (cardValue >= maxValue)
                {
                    maxValue = cardValue;
                    max = i;
                }
                //Console.Error.WriteLine(cardList[i].cardNumber + " " + cardValue);
            }
            CurveAdd(myHand[max].Cost, curve);
            return "PICK " + max;
        }

        public static Card GetMostExpensiveCard(int mana, int myBoardCount, int enemyBoardCount, List<Card> myHand)
        {
            foreach (Card card in myHand)
            {
                //Console.Error.WriteLine(card.cost);
                int maxCost = 0;
                if (card.Cost <= mana)
                {
                    if (card.CardType == 0 && myBoardCount < 6 || card.CardType == 3 || card.CardType == 1 && myBoardCount != 0 || card.CardType == 2 && enemyBoardCount != 0)
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

        public static string GetBestSummon(int turn, List<Card> enemyBoard, List<Card> myHand, List<Card> myBoard)
        {
            int mana = turn - 30;
            int boardCount = myBoard.Count;
            int enemyBoardCount = myBoard.Count;
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
                if (card.CardType == 1)
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
                if (card.CardType == 2 || card.CardType == 3 && card.Defense < 0)
                {
                    if (enemyBoard.Count == 0)
                    {
                        target += -1;
                    }
                    else
                    {
                        target += enemyBoard[0].InstanceId;
                        if (enemyBoard[0].Defense <= -card.Defense)
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
                        enemyTreat[0].Defense = 0;
                    }
                    else
                    {
                        enemyTreat[0].Defense -= card.Attack;
                    }
                    if (enemyTreat[0].Defense <= 0)
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
