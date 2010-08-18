package
{
	import flash.display.Sprite;
	import flash.utils.Dictionary;
	
	public class DictionaryTest extends Sprite
	{
		public function DictionaryTest()
		{
			var testDict:Dictionary = new Dictionary;
			testDict["foo"] = "test1";
			testDict["bar"] = 3.14;
			testDict["baz"] = testDict;
			
			//Test iteration
			for each(var str:String in testDict)
			{
				trace(str);
			}
		}
	}
}