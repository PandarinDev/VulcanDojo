using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using CCG;
using System.Collections.Generic;
using System.IO;

namespace CCGBot.Tests
{
    [TestClass]
    public class SimulatorTests
    {
        [TestMethod]
        public void Test_GetMostExpensiveCard()
        {
            string input = string.Join("\n", new string[]
            {// CID, IID, Loc, Type, Cost, Att, Def, Abl, HP, EnemyHP, Draw
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
           Assert.AreEqual(cards[1], BattlePhase.GetMostExpensiveCard(8, 0, 0, cards));
        }

        [TestMethod]
        public void SimulateSimpleAttack()
        {
            var attacker = Parse.Card("120 1  1 1 2 1 2 ------ 0 0 0");
            var defender = Parse.Card("121 2 -1 1 2 2 2 ------ 0 0 0");
            Assert.AreEqual(2, attacker.DefenseValue);
            Assert.AreEqual(2, defender.DefenseValue);

            Simulator.Attack(attacker, defender);

            Assert.AreEqual(0, attacker.DefenseValue);
            Assert.AreEqual(1, defender.DefenseValue);
        }

        [TestMethod]
        public void SimulateSimpleAttackTwice()
        {
            var attacker1 = Parse.Card("120 1  1 1 2 1 2 ------ 0 0 0");
            var attacker2 = Parse.Card("120 1  1 1 2 1 2 ------ 0 0 0");
            var defender0 = Parse.Card("121 2 -1 1 2 2 2 ------ 0 0 0");
            Assert.AreEqual(2, attacker1.DefenseValue);
            Assert.AreEqual(2, attacker2.DefenseValue);
            Assert.AreEqual(2, defender0.DefenseValue);

            Simulator.Attack(attacker1, defender0);
            Simulator.Attack(attacker2, defender0);

            Assert.AreEqual(0, attacker1.DefenseValue);
            Assert.AreEqual(2, attacker2.DefenseValue);
            Assert.AreEqual(0, defender0.DefenseValue);
        }
    }
}
