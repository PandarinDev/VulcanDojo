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
    public static int GetBestCard(List<Card> cards, List<Card> deck)
    {
        var other = new List<Card>(cards);
        
        other = other.OrderBy(c => c.attack).ToList();
        if (Util.avg(deck) > 2.5)
        {
            other = other.OrderBy(c => c.cost).ToList();
        }
        else
        {
            other = other.OrderByDescending (c => c.cost).ToList();
        }
        return cards.IndexOf(other[0]);
    }

    public static float avg(List<Card> deck)
    {
        float av = 0;
        foreach (var card in deck)
        {
            av += card.cost;
        }
        if (deck.Count > 0)
            av /= deck.Count;
        return av;
    }

    public static Card MaxAttack(List<Card> deck)
    {
        int max = 0;
        Card c = null;
        foreach (var card in deck)
        {
            if (card.attack > max)
            {
                max += card.attack;
                c = card;
            }
        }
        return c;
    }

    public static int GetBestSummon(List<Card> hand, ref int mana)
    {
        hand.OrderByDescending(c => c.cost);
        foreach(var card in hand)
        {
            if(card.cost <= mana)
            {
                mana -= card.cost;
                return card.instanceId;
            }
        }
        return -1;
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
        var cards = new List<Card>();
        var deck = new List<Card>();
        var costs = new List<int>();
        var hand = new List<Card>();
        var cardsInBoard = new List<Card>();
        Stack<int> targets = new Stack<int>();
        string command = "";
        string board = "";
        int playerMana = 0;
        string attackcommand = "";
        int cardCountInBoard = 0;
        int turn = 0;
        // game loop
        while (true)
        {

            for (int i = 0; i < 2; i++)
            {
                inputs = Console.ReadLine().Split(' ');
                int playerHealth = int.Parse(inputs[0]);
                playerMana = int.Parse(inputs[1]);
                int playerDeck = int.Parse(inputs[2]);
                int playerRune = int.Parse(inputs[3]);
            }
            int opponentHand = int.Parse(Console.ReadLine());
            int cardCount = int.Parse(Console.ReadLine());

            cards.Clear();
            for (int i = 0; i < cardCount; i++)
            {
                inputs = Console.ReadLine().Split(' ');
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
                cards.Add(card);
                
                if (turn > 30)
                {
                    if (card.location == 0)
                    {
                        hand.Add(card);
                    }
                    //Target:
                    if (card.location == -1 && card.abilities.Contains("G"))
                    {
                        targets.Push(card.instanceId);
                    }
                    else if(card.location == 1)
                    {
                        cardsInBoard.Add(card);
                        cardCountInBoard++;
                    }
                    //  Console.Error.WriteLine(card.instanceId);
                    //  Console.Error.WriteLine(card.cost);




                }
            }

            //draft phase
            if (turn < 30)
            {
                var bestIndex = Util.GetBestCard(cards, deck);
                Console.WriteLine("PICK " + bestIndex);

                costs.Add(cards[bestIndex].cost);
                deck.Add(cards[bestIndex]);
            }
            else
            {
                //summon
                int actMana = playerMana;
                int summon;
                do
                {
                    summon = Util.GetBestSummon(hand, ref playerMana);
                    cardsInBoard.Add(hand.Find(c => c.instanceId == summon));
                    hand.Remove(hand.Find(c => c.instanceId == summon));
                    if (summon != -1 && cardCountInBoard < 6)
                    {
                        command += "SUMMON " + summon + ";";
                    }
                }
                while (summon != -1 && cardCountInBoard < 6);

                //attack
                //Attack:
                foreach (Card card in cardsInBoard)
                {

                    if (targets.Count != 0)
                    { 
                        attackcommand += "ATTACK " + card.instanceId + " " + targets.Pop() + ";";
                    }
                    else
                    {
                        attackcommand += "ATTACK " + card.instanceId + " -1;";
                    }
                }
            
                if (command == "" && board == "")
                {
                    command = "PASS";
                }
                Console.WriteLine(command + board);
                command = "";
            }
            ++turn;

        }
    }
}