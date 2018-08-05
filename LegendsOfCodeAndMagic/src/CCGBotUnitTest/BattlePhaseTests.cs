using System;
using System.Collections.Generic;
using System.IO;
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
        public void PossibleAction_NoAction()
        {
            Queue<string> stateStrings = new Queue<string>
            {
                ("30 2 24 25"), ("30 2 24 25"), "6", "1",
                "69 3 0 0 3 4 4  ------ 0 0 0",
            };
            GameState gs = Parse.GameState(stateStrings);

            List<GameAction> actions = BattlePhase.GraphSolver.GetPossibleActions(gs);

            Assert.AreEqual(1, actions.Count);
            Assert.AreEqual(ActionType.NoAction, actions[0].Type);
        }

        #region CreatureAttack action tests
        [TestMethod]
        public void PossibleAction_Attack()
        {
            GameState gs = Parse.GameState(new Queue<string>
            {
                ("30 2 24 25"), ("30 2 24 25"), "6", "1",
                "69 3 1 0 3 4 4  ------ 0 0 0",
            });

            List<GameAction> actions = BattlePhase.GraphSolver.GetPossibleActions(gs);

            Assert.AreEqual(2, actions.Count);
            Assert.AreEqual(ActionType.NoAction, actions[0].Type);
            Assert.AreEqual(ActionType.CreatureAttack, actions[1].Type);
        }

        [TestMethod]
        public void PossibleAction_AttackWithTwo()
        {
            GameState gs = Parse.GameState(new Queue<string>
            {
                ("30 2 24 25"), ("30 2 24 25"), "6", "2",
                "69 3 1 0 3 4 4  ------ 0 0 0",
                "70 2 1 0 3 2 2  ------ 0 0 0",
            });

            List<GameAction> actions = BattlePhase.GraphSolver.GetPossibleActions(gs);

            Assert.AreEqual(3, actions.Count);
            Assert.AreEqual(ActionType.NoAction, actions[0].Type);
            Assert.AreEqual(ActionType.CreatureAttack, actions[1].Type);
            Assert.AreEqual(ActionType.CreatureAttack, actions[2].Type);
        }

        [TestMethod]
        public void PossibleAction_AttackTwoTargets()
        {
            GameState gs = Parse.GameState(new Queue<string>
            {
                ("30 2 24 25"), ("30 2 24 25"), "6", "2",
                "69 3 1 0 3 4 4  ------ 0 0 0",
                "70 2 -1 0 3 2 2  ------ 0 0 0",
            });

            List<GameAction> actions = BattlePhase.GraphSolver.GetPossibleActions(gs);

            Assert.AreEqual(3, actions.Count);
            Assert.AreEqual(ActionType.NoAction, actions[0].Type);
            Assert.AreEqual(ActionType.CreatureAttack, actions[1].Type);
            Assert.AreEqual(3, actions[1].Id);
            Assert.AreEqual(-1, actions[1].TargetId);
            Assert.AreEqual(ActionType.CreatureAttack, actions[2].Type);
            Assert.AreEqual(3, actions[2].Id);
            Assert.AreEqual(2, actions[2].TargetId);
        }

        [TestMethod]
        public void PossibleAction_AttackGuard()
        {
            GameState gs = Parse.GameState(new Queue<string>
            {
                ("30 2 24 25"), ("30 2 24 25"), "6", "2",
                "69 3 1 0 3 4 4  ------ 0 0 0",
                "70 2 -1 0 3 2 2  ---G-- 0 0 0",
            });

            List<GameAction> actions = BattlePhase.GraphSolver.GetPossibleActions(gs);

            Assert.AreEqual(2, actions.Count);
            Assert.AreEqual(ActionType.NoAction, actions[0].Type);
            Assert.AreEqual(ActionType.CreatureAttack, actions[1].Type);
            Assert.AreEqual(3, actions[1].Id);
            Assert.AreEqual(2, actions[1].TargetId);
        }

        [TestMethod]
        public void PossibleAction_AttackOnlyGuard()
        {
            GameState gs = Parse.GameState(new Queue<string>
            {
                ("30 2 24 25"), ("30 2 24 25"), "6", "3",
                "69 3 1 0 3 4 4  ------ 0 0 0",
                "70 2 -1 0 3 2 2  ---G-- 0 0 0",
                "71 4 -1 0 4 5 2  ------ 0 0 0",
            });

            List<GameAction> actions = BattlePhase.GraphSolver.GetPossibleActions(gs);

            Assert.AreEqual(2, actions.Count);
            Assert.AreEqual(ActionType.NoAction, actions[0].Type);
            Assert.AreEqual(ActionType.CreatureAttack, actions[1].Type);
            Assert.AreEqual(3, actions[1].Id);
            Assert.AreEqual(2, actions[1].TargetId);
        }

        #endregion

        #region PlayCard action tests
        [TestMethod]
        public void PossibleAction_PlayCard()
        {
            GameState gs = Parse.GameState(new Queue<string>
            {
                ("30 4 24 25"), ("30 4 24 25"), "6", "1",
                "69 3 0 0 3 4 4  ------ 0 0 0",
            });

            List<GameAction> actions = BattlePhase.GraphSolver.GetPossibleActions(gs);

            Assert.AreEqual(2, actions.Count);
            Assert.AreEqual(ActionType.NoAction, actions[0].Type);
            Assert.AreEqual(ActionType.PlayCard, actions[1].Type);
            Assert.AreEqual(3, actions[1].Id);
        }

        [TestMethod]
        public void PossibleAction_PlayMoreCards()
        {
            GameState gs = Parse.GameState(new Queue<string>
            {
                ("30 4 24 25"), ("30 4 24 25"), "6", "2",
                "69 3 0 0 3 4 4  ------ 0 0 0",
                "70 2 0 0 2 2 2  ------ 0 0 0",
            });

            List<GameAction> actions = BattlePhase.GraphSolver.GetPossibleActions(gs);

            Assert.AreEqual(3, actions.Count);
            Assert.AreEqual(ActionType.NoAction, actions[0].Type);
            Assert.AreEqual(ActionType.PlayCard, actions[1].Type);
            Assert.AreEqual(3, actions[1].Id);
            Assert.AreEqual(ActionType.PlayCard, actions[2].Type);
            Assert.AreEqual(2, actions[2].Id);
        }
        #endregion

        #region UseItem action tests

        [TestMethod]
        public void PossibleAction_UseGreenItem()
        {
            GameState gs = Parse.GameState(new Queue<string>
            {
                ("30 4 24 25"), ("30 4 24 25"), "6", "2",
                "119 1 0 1 1 1 2  ------ 0 0 0",
                "70 2 1 0 2 2 2  ------ 0 0 0",
            });

            List<GameAction> actions = BattlePhase.GraphSolver.GetPossibleActions(gs);

            Assert.AreEqual(3, actions.Count);
            Assert.AreEqual(ActionType.NoAction, actions[0].Type);
            Assert.AreEqual(ActionType.CreatureAttack, actions[1].Type);
            Assert.AreEqual(2, actions[1].Id);
            Assert.AreEqual(-1, actions[1].TargetId);
            Assert.AreEqual(ActionType.UseItem, actions[2].Type);
            Assert.AreEqual(1, actions[2].Id);
            Assert.AreEqual(2, actions[2].TargetId);
        }

        [TestMethod]
        public void PossibleAction_UseRedItem()
        {
            GameState gs = Parse.GameState(new Queue<string>
            {
                ("30 4 24 25"), ("30 4 24 25"), "6", "2",
                "145 1 0 2 3 -2 -2 ------ 0 0 0",
                "70 2 -1 0 2 2 2  ------ 0 0 0",
            });

            List<GameAction> actions = BattlePhase.GraphSolver.GetPossibleActions(gs);

            Assert.AreEqual(2, actions.Count);
            Assert.AreEqual(ActionType.NoAction, actions[0].Type);
            Assert.AreEqual(ActionType.UseItem, actions[1].Type);
            Assert.AreEqual(1, actions[1].Id);
            Assert.AreEqual(2, actions[1].TargetId);
        }

        [TestMethod]
        public void PossibleAction_UseBlueItem()
        {
            GameState gs = Parse.GameState(new Queue<string>
            {
                ("30 4 24 25"), ("30 4 24 25"), "6", "1",
                "157 1 0 3 3 0 -1 ------ 1 0 1",
            });

            List<GameAction> actions = BattlePhase.GraphSolver.GetPossibleActions(gs);

            Assert.AreEqual(2, actions.Count);
            Assert.AreEqual(ActionType.NoAction, actions[0].Type);
            Assert.AreEqual(ActionType.UseItem, actions[1].Type);
            Assert.AreEqual(1, actions[1].Id);
            Assert.AreEqual(-1, actions[1].TargetId);
        }

        [TestMethod]
        public void PossibleAction_UseBlueItemOnCreature()
        {
            GameState gs = Parse.GameState(new Queue<string>
            {
                ("30 4 24 25"), ("30 4 24 25"), "6", "2",
                "157 1 0 3 3 0 -1 ------ 1 0 1",
                "70 2 -1 0 2 2 2  ------ 0 0 0",
            });

            List<GameAction> actions = BattlePhase.GraphSolver.GetPossibleActions(gs);

            Assert.AreEqual(3, actions.Count);
            Assert.AreEqual(ActionType.NoAction, actions[0].Type);
            Assert.AreEqual(ActionType.UseItem, actions[1].Type);
            Assert.AreEqual(1, actions[1].Id);
            Assert.AreEqual(2, actions[1].TargetId);
            Assert.AreEqual(ActionType.UseItem, actions[2].Type);
            Assert.AreEqual(1, actions[2].Id);
            Assert.AreEqual(-1, actions[2].TargetId);
        }

        [TestMethod]
        public void PossibleAction_UseBlueItemCantTargetCreature()
        {
            GameState gs = Parse.GameState(new Queue<string>
            {
                ("30 4 24 25"), ("30 4 24 25"), "6", "2",
                "154 1 0 3 2 0 0 ------ 0 -2 1",
                "70 2 -1 0 2 2 2 ------ 0 0 0",
            });

            List<GameAction> actions = BattlePhase.GraphSolver.GetPossibleActions(gs);

            Assert.AreEqual(2, actions.Count);
            Assert.AreEqual(ActionType.NoAction, actions[0].Type);
            Assert.AreEqual(ActionType.UseItem, actions[1].Type);
            Assert.AreEqual(1, actions[1].Id);
            Assert.AreEqual(-1, actions[1].TargetId);
        }

        // TODO: Dont use green item on enemy
        // TODO: Dont use red item on player

        #endregion

        // TODO: Combined tests with more creatures on board and cards in hand
    }

    public static class Extensions
    {
        public static void Add<T>(this Queue<T> list, T item) =>list.Enqueue(item);
    }
}
