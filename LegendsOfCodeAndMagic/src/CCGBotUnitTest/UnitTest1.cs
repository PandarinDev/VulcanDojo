using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using CCG;
using System.Collections.Generic;

namespace CCGBotUnitTest
{
    [TestClass]
    public class UnitTest1
    {
        [TestMethod]
        public void Test_GetMostExpensiveCard()
        {
            List<Card> cards = new List<Card>()
           {
               new Card(0, 0, 0, 0, 5, 0, 0, string.Empty, 0, 0, 0),
               new Card(1, 0, 0, 0, 4, 0, 0, string.Empty, 0, 0, 0),
               new Card(2, 0, 0, 0, 3, 0, 0, string.Empty, 0, 0, 0),
               new Card(2, 0, 3, 0, 2, 0, 0, string.Empty, 0, 0, 0),
               new Card(2, 0, 1, 0, 3, 0, 0, string.Empty, 0, 0, 0),
           };
           Assert.AreEqual(cards[0], Util.GetMostExpensiveCard(5, 0, 0, cards));
           Assert.AreEqual(cards[3], Util.GetMostExpensiveCard(5, 6, 0, cards));
           Assert.AreEqual(cards[1], Util.GetMostExpensiveCard(4, 5, 0, cards));
           Assert.AreEqual(cards[4], Util.GetMostExpensiveCard(5, 6, 0, cards));
        }

        [TestMethod]
        public void Test_GetBestCard()
        {

        }
    }
}
