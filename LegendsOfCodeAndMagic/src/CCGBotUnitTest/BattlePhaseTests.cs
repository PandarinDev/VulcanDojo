﻿using System;
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
        public void PossibleActioNoAction()
        {
            Queue<string> stateStrings = new Queue<string>
            {
                ("30 2 24 25"), ("30 2 24 25"), "6", "1",
                "69 3 0 0 3 4 4  B----- 0 0 0",
            };
            GameState gs = Parse.GameState(stateStrings);

            List<GameAction> actions = BattlePhase.GraphSolver.GetPossibleActions(gs);

            Assert.AreEqual(1, actions.Count);
            Assert.AreEqual(ActionType.NoAction, actions[0].Type);
        }

        [TestMethod]
        public void PossibleActionAttack()
        {
            GameState gs = Parse.GameState(new Queue<string>
            {
                ("30 2 24 25"), ("30 2 24 25"), "6", "1",
                "69 3 1 0 3 4 4  B----- 0 0 0",
            });

            List<GameAction> actions = BattlePhase.GraphSolver.GetPossibleActions(gs);

            Assert.AreEqual(2, actions.Count);
            Assert.AreEqual(ActionType.NoAction, actions[0].Type);
            Assert.AreEqual(ActionType.CreateAttackAction, actions[1].Type);
        }
    }

    public static class Extensions
    {
        public static void Add<T>(this Queue<T> list, T item) =>list.Enqueue(item);
    }
}
