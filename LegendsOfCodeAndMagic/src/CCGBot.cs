using System;
using System.Linq;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
class Player
{
    static void Main(string[] args)
    {
        string[] inputs;
        int turn = 0;
        string command = "";
        string board = "";
        int tempcost = 0;

        // game loop
        while (true)
        {
            for (int i = 0; i < 2; i++)
            {
                inputs = Console.ReadLine().Split(' ');
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
                int cardNumber = int.Parse(inputs[0]);
                int instanceId = int.Parse(inputs[1]);
                int location = int.Parse(inputs[2]);
                int cardType = int.Parse(inputs[3]);
                int cost = int.Parse(inputs[4]);
                int attack = int.Parse(inputs[5]);
                int defense = int.Parse(inputs[6]);
                string abilities = inputs[7];
                int myHealthChange = int.Parse(inputs[8]);
                int opponentHealthChange = int.Parse(inputs[9]);
                int cardDraw = int.Parse(inputs[10]);
                //Console.Error.WriteLine(draft);
                if (turn > 30) {
                Console.Error.WriteLine(instanceId);
                Console.Error.WriteLine(cost);
                    if (cost <= turn - 29 - tempcost){
                        command += "SUMMON " + instanceId + ";";
                        board +=  "ATTACK " + instanceId + " -1;";
                        tempcost += cost;
                    }
                }
            }
            tempcost = 0;
            ++turn;

            // Write an action using Console.WriteLine()
            if (command == "" && board == ""){
                command = "PASS";
            }
            Console.WriteLine(command+board);
            command = "";
        }
    }
}