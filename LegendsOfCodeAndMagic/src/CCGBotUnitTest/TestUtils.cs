using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CCG.Tests
{
    class TestUtils
    {
    }
    
    public static class AssertExt
    {
        public static void Add<T>(this Queue<T> list, T item) => list.Enqueue(item);

        public static void HasAnyItem<T>(IEnumerable<T> items, Func<T, bool> func)
        {
            Assert.IsTrue(items.Any(t => func(t)), "Didn't found an item which fulfilled the conditions.");
        }

        public static void HasUniqueItem<T>(List<T> items, Func<T, bool> func)
        {
            var it = items.FindAll(i => func(i));
            Assert.AreEqual(1, it.Count, "There are no item, or more than one items which fulfill the conditions.");
        }

        public static void HasNoItemWithCondition<T>(IEnumerable<T> items, Func<T, bool> func)
        {
            Assert.IsFalse(items.Any(t => func(t)), "Found an item which fulfilled the conditions.");
        }
    }
}
