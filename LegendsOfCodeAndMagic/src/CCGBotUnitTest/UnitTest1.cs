using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using CCG;
using System.Collections.Generic;
using System.IO;

namespace CCGBotUnitTest
{
    [TestClass]
    public class UnitTest1
    {
        [TestMethod]
        public void Test_GetMostExpensiveCard()
        {
            string input = @"120 -1 0 1 2 1 0 ----L- 0 0 0
90 -1 0 0 8 5 5 -C---- 0 0 0
60 -1 0 0 7 4 8 ------ 0 0 0";
            StringReader strReader = new StringReader(input);
            List<Card> cards = new List<Card>()
           {
               Util.SetCardValue(strReader.ReadLine()),
               Util.SetCardValue(strReader.ReadLine()),
               Util.SetCardValue(strReader.ReadLine())
           };
           Assert.AreEqual(cards[1], Util.GetMostExpensiveCard(8, 0, 0, cards));
        }

        [TestMethod]
        public void Test_GetBestCard()
        {

        }
    }
}
