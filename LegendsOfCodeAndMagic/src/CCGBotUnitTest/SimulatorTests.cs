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
            SimulateAttack(1, 2,  2, 2,  0, 1);
            SimulateAttack(3, 2,  1, 2,  1, -1);
            SimulateAttack(0, 2,  1, 2,  1, 2);
            SimulateAttack(1, 2,  0, 2,  2, 1);
            SimulateAttack(2, 2,  2, 2,  0, 0);
        }

        private static void SimulateAttack(int attackerAtt, int attackerDef, 
            int defenderAtt, int defenderDef, int attackerAfterDef, int defenderAfterDef)
        {
            var attacker = Parse.Card($"120 1  1 1 2 {attackerAtt} {attackerDef} ------ 0 0 0");
            var defender = Parse.Card($"121 2 -1 1 2 {defenderAtt} {defenderDef} ------ 0 0 0");
            Assert.AreEqual(attackerDef, attacker.DefenseValue);
            Assert.AreEqual(defenderDef, defender.DefenseValue);

            Simulator.Attack(attacker, defender);

            Assert.AreEqual(attackerAfterDef, attacker.DefenseValue);
            Assert.AreEqual(defenderAfterDef, defender.DefenseValue);
        }

        [TestMethod]
        public void SimulateSimpleAttackTwice()
        {
            var attacker1 = Parse.Card("120 1  1 1 2 1 2 ------ 0 0 0");
            var attacker2 = Parse.Card("120 1  1 1 2 1 2 ------ 0 0 0");
            var defender0 = Parse.Card("121 2 -1 1 2 2 2 ------ 0 0 0");

            Simulator.Attack(attacker1, defender0);
            Simulator.Attack(attacker2, defender0);

            Assert.AreEqual(0, attacker1.DefenseValue);
            Assert.AreEqual(2, attacker2.DefenseValue);
            Assert.AreEqual(0, defender0.DefenseValue);
        }

        [TestMethod]
        public void SimulateWardAttack()
        {
            var attacker1 = Parse.Card("120 1  1 1 2 1 2 ------ 0 0 0");
            var defender0 = Parse.Card("120 1  1 1 2 2 2 -----W 0 0 0");

            Simulator.Attack(attacker1, defender0);

            Assert.AreEqual(0, attacker1.DefenseValue);
            Assert.AreEqual(2, defender0.DefenseValue);
        }

        [TestMethod]
        public void SimulateWardAttack0Damage()
        {
            var attacker1 = Parse.Card("120 1  1 1 2 0 2 ------ 0 0 0");
            var defender0 = Parse.Card("120 1  1 1 2 2 2 -----W 0 0 0");

            Simulator.Attack(attacker1, defender0);

            Assert.AreEqual(0, attacker1.DefenseValue);
            Assert.AreEqual(2, defender0.DefenseValue);
            Assert.IsTrue(defender0.HasWard());
        }

        [TestMethod]
        public void SimulateWardAttackTwice()
        {
            var attacker1 = Parse.Card("120 1  1 1 2 1 2 ------ 0 0 0");
            var attacker2 = Parse.Card("120 1  1 1 2 1 2 ------ 0 0 0");
            var defender0 = Parse.Card("120 1  1 1 2 2 2 -----W 0 0 0");

            Simulator.Attack(attacker1, defender0);
            Simulator.Attack(attacker2, defender0);

            Assert.AreEqual(0, attacker1.DefenseValue);
            Assert.AreEqual(2, attacker2.DefenseValue);
            Assert.AreEqual(1, defender0.DefenseValue);
        }

        [TestMethod]
        public void SimulateWithWardAttack()
        {
            var attacker1 = Parse.Card("120 1  1 1 2 1 2 -----W 0 0 0");
            var defender0 = Parse.Card("120 1  1 1 2 2 2 ------ 0 0 0");
            Assert.IsTrue(attacker1.HasWard());

            Simulator.Attack(attacker1, defender0);

            Assert.AreEqual(2, attacker1.DefenseValue);
            Assert.IsFalse(attacker1.HasWard());
            Assert.AreEqual(1, defender0.DefenseValue);
        }

        [TestMethod]
        public void SimulateWithWardAttack0Damage()
        {
            var attacker1 = Parse.Card("120 1  1 1 2 1 2 -----W 0 0 0");
            var defender0 = Parse.Card("120 1  1 1 2 0 2 ------ 0 0 0");
            Assert.IsTrue(attacker1.HasWard());

            Simulator.Attack(attacker1, defender0);

            Assert.AreEqual(2, attacker1.DefenseValue);
            Assert.IsTrue(attacker1.HasWard());
            Assert.AreEqual(1, defender0.DefenseValue);
        }

        [TestMethod]
        public void SimulateWithLethalAttack()
        {
            var attacker1 = Parse.Card("120 1  1 1 2 1 2 ----L- 0 0 0");
            var defender0 = Parse.Card("120 1  1 1 2 1 2 ------ 0 0 0");

            Simulator.Attack(attacker1, defender0);

            Assert.AreEqual(1, attacker1.DefenseValue);
            Assert.AreEqual(0, defender0.DefenseValue);
        }

        [TestMethod]
        public void SimulateWardWithLethalAttack()
        {
            var attacker1 = Parse.Card("120 1  1 1 2 1 2 ----L- 0 0 0");
            var defender0 = Parse.Card("120 1  1 1 2 1 2 -----W 0 0 0");

            Simulator.Attack(attacker1, defender0);

            Assert.AreEqual(1, attacker1.DefenseValue);
            Assert.AreEqual(2, defender0.DefenseValue);
            Assert.IsFalse(defender0.HasWard());
        }
    }
}
