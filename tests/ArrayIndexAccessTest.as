package
{
	import flash.display.Sprite;
	
	public class ArrayIndexAccessTest extends Sprite
	{
		public function ArrayIndexAccessTest()
		{
			var array:Array=new Array;
			array.push("test1");
			array.push(2.5);
			array.push("test3");
			
			trace(array[2]);
		}
	}
}