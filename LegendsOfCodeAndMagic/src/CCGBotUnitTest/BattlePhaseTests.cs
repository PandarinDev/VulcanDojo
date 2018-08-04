using System;
using System.Collections.Generic;
using System.IO;
using CCG;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace CCG.Tests
{
    [TestClass]
    public class BattlePhaseTests
    {
        // CID, IID, Loc, Type, Cost, Att, Def, Abl, HP, EnemyHP, Draw
        /*  11 1 0 0 3 5 2    ------ 0 0 0 
            69 3 0 0 3 4 4    B----- 0 0 0 
            56 5 0 0 4 2 7    ------ 0 0 0 
            117 7 0 1 1 1 1   B----- 0 0 0 
            119 9 0 1 1 1 2   ------ 0 0 0 
            148 11 0 2 2 0 -2 BCDGLW 0 0 0
         */
        [TestMethod]
        public void ZeroPossibleActions()
        {
            Queue<string> stateStrings = new List<string>
            {
                "30 2 24 25",
                "30 2 24 25",
                "6", "1",
                "69 3 0 0 3 4 4  B----- 0 0 0",
            }.ToQueue();
            GameState gs = Parse.GameState(stateStrings);

            List<GameAction> actions = BattlePhase.GraphSolver.GetPossibleActions(gs);

            Assert.AreEqual(1, actions.Count);
            Assert.AreEqual(ActionType.NoAction, actions[0].Type);
        }


        [TestMethod]
        public void Test_GetMostExpensiveCard()
        {
            // CID, IID, Loc, Type, Cost, Att, Def, Abl, HP, EnemyHP, Draw
            // example Creature: "148 11 0 2 2 0 -2 BCDGLW 0 0 0"
            string input = string.Join("\n", new string[]
            {
                @"120 -1 0 1 2 1 0 ----L- 0 0 0",
                @"90 -1 0 0 8 5 5 -C---- 0 0 0",
                @"60 -1 0 0 7 4 8 ------ 0 0 0"
            });
            StringReader strReader = new StringReader(input);
            List<Card> cards = new List<Card>()
            {
                Parse.Card(strReader.ReadLine()),
                Parse.Card(strReader.ReadLine()),
                Parse.Card(strReader.ReadLine())
            };
            Assert.AreEqual(cards[1], BattlePhase.HumanSolver.GetMostExpensiveCard(8, 0, 0, cards));
        }
    }

    public static class Extensions
    {
        public static Queue<T> ToQueue<T>(this List<T> list)
        {
            Queue<T> result = new Queue<T>();
            foreach (var item in list)
            {
                result.Enqueue(item);
            }
            return result;
        }
    }
}
