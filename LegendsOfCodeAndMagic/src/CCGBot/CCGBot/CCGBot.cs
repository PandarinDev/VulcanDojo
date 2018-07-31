using System;
using System.Linq;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;

public class Card
{
    public int cardNumber;
    public int instanceId;
    public int location;
    public int cardType;
    public int cost;
    public int attack;
    public int defense;
    public string abilities;
    public int myHealthChange;
    public int opponentHealthChange;
    public int cardDraw;
}

public static class Util
{
    public static double GetValue(Card card, int[] curve)
    {
        //TODO: improve red and blue item values
        double value = 0;
        value += Math.Abs(card.attack);
        value += Math.Abs(card.defense);
        value += card.cardDraw;
        value += card.abilities.Replace("-", "").Replace("L", "L2").Replace("W", "W2").Length * 0.5;
        value += card.myHealthChange / 3;
        value -= card.opponentHealthChange / 3;
        value -= card.cost * 2;
        //marginal penalty
        if (card.cost == 0 || card.attack == 0)
        {
            value -= 2;
        }
        //nonboard penalty
        if (card.cardType == 2 || card.cardType == 3)
        {
            value -= 1;
        }
        //balance penalty, nerf decimate
        if (card.cardNumber == 155)
        {
            value -= 94;
        }
        if (GetCurveFit(card.cost, curve))
        {
            --value;
        }
        if (card.attack - card.defense >= 2)
        {
            value += 0.01;
        }
        if (-card.attack + card.defense >= 2)
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
        CurveAdd(myHand[max].cost, curve);
        return "PICK " + max;

    }
    private static Card GetMostExpensiveCard(int mana, int myBoardCount, int enemyBoardCount, List<Card> myHand)
    {
        foreach (Card card in myHand)
        {
            //Console.Error.WriteLine(card.cost);
            int maxCost = 0;
            if (card.cost <= mana)
            {
                if (card.cardType == 0 && myBoardCount < 6 || card.cardType == 3 || card.cardType == 1 && myBoardCount != 0 || card.cardType == 2 && enemyBoardCount != 0)
                {
                    if (maxCost <= card.cost)
                    {
                        maxCost = card.cost;
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
            if (card.cardType == 1)
            {
                if (myBoard.Count != 0)
                {
                    target += myBoard[0].instanceId;
                }
                else
                {
                    target = (command.Split(' '))[1];
                }
            }
            //TODO: improve debuff items
            if (card.cardType == 2 || card.cardType == 3 && card.defense < 0)
            {
                if (enemyBoard.Count == 0)
                {
                    target += -1;
                }
                else
                {
                    target += enemyBoard[0].instanceId;
                    if (enemyBoard[0].defense <= -card.defense)
                    {
                        enemyBoard.RemoveAt(0);
                    }
                }
            }
            if (card.cardType == 0)
            {
                ++boardCount;
                if (card.abilities.Contains("C"))
                {
                    myBoard.Add(card);
                }
                command += "SUMMON " + card.instanceId + ";";
            }
            else
            {
                command += "USE " + card.instanceId + " " + target + ";";
            }
            target = "";
            mana -= card.cost;
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
            if (card.abilities.Contains("G"))
            {
                enemyTreat.Add(card);
            }
        }
        foreach (Card card in myBoard)
        {
            if (enemyTreat.Count != 0)
            {
                attacks += "ATTACK " + card.instanceId + " " + enemyTreat[0].instanceId + ";";
                if (card.abilities.Contains("L"))
                {
                    enemyTreat[0].defense = 0;
                }
                else
                {
                    enemyTreat[0].defense -= card.attack;
                }
                if (enemyTreat[0].defense <= 0)
                {
                    enemyTreat.RemoveAt(0);
                }
            }
            else
            {
                attacks += "ATTACK " + card.instanceId + " -1;";
            }
        }

        return attacks;
    }
}
/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
class Player
{
    static void Main(string[] args)
    {
        string[] inputs;
        var enemyBoard = new List<Card>();
        var myHand = new List<Card>();
        var myBoard = new List<Card>();
        int turn = 0;
        int[] curve = new int[] { 2, 8, 7, 5, 4, 2, 2 };

        // game loop
        while (true)
        {
            for (int i = 0; i < 2; i++)
            {
                inputs = Console.ReadLine().Split(' ');
                Console.Error.WriteLine(String.Join(" ", inputs));
                int playerHealth = int.Parse(inputs[0]);
                int playerMana = int.Parse(inputs[1]);
                int playerDeck = int.Parse(inputs[2]);
                int playerRune = int.Parse(inputs[3]);
            }
            int opponentHand = int.Parse(Console.ReadLine());
            int cardCount = int.Parse(Console.ReadLine());
            for (int i = 0; i < cardCount; i++)
            {
                inputs = Console.ReadLine().Split(' ');
                Console.Error.WriteLine(String.Join(" ", inputs));
                var card = new Card
                {
                    cardNumber = int.Parse(inputs[0]),
                    instanceId = int.Parse(inputs[1]),
                    location = int.Parse(inputs[2]),
                    cardType = int.Parse(inputs[3]),
                    cost = int.Parse(inputs[4]),
                    attack = int.Parse(inputs[5]),
                    defense = int.Parse(inputs[6]),
                    abilities = inputs[7],
                    myHealthChange = int.Parse(inputs[8]),
                    opponentHealthChange = int.Parse(inputs[9]),
                    cardDraw = int.Parse(inputs[10])
                };

                //Classify cards given their location
                // if draft phase: store it in draft stack, process value and choose
                // if game phase: if enemy board decide what to trade
                //                if hand decide what to play
                //                if my board decide where to hit
                // draft is when turn < 30
                switch (card.location)
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
                //if(playHand.Count() != 0){Console.Error.WriteLine(playHand[0].attack);}



            }
            //modify curve
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
