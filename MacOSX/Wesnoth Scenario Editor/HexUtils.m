//
//  HexUtils.m
//  Wesnoth Scenario Editor
//
//  Created by Marcus on Fri Mar 26 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "HexUtils.h"


@implementation HexUtils
	int HexagonH=0;
	int HexagonS=0;
	int TileR=0;
	float m;
	
+(void)initWithWidth: (int)width
{
	TileR = width/2;
	HexagonH = TileR*(0.5773502691896257);// Tan 30 :D
	HexagonS = width - (HexagonH*2);
	m = HexagonH / TileR;
}
	
+(NSPoint)hexFromX:(int)PixelX y:(int)PixelY
{
	int SectX=0, SectY=0, SectPxlX=0, SectPxlY=0, ArrayX=0,ArrayY=0;
	NSPoint myPoint;
	
	SectX = PixelX / (2 * TileR);
	SectY = PixelY / (HexagonH + HexagonS);
	
	SectPxlX = PixelX % (2 * TileR);
	SectPxlY = PixelY % (HexagonH + HexagonS);
	
	if ((SectY & 1)==0)
		{   //  Type A
		ArrayY = SectY;
		ArrayX = SectX;
		// Left edge
		if (SectPxlY < (HexagonH - SectPxlX*m))
			{
			ArrayY = SectY - 1;
			ArrayX = SectX - 1;
			}
		// Right edge
		if (SectPxlY<(SectPxlX*m-HexagonH))
			{
			ArrayY = SectY - 1;
			ArrayX = SectX;
			}
		}else{  // Type B
		//right side
		if (SectPxlX >= TileR)
			{
			if (SectPxlY < ((2 * HexagonH) - (SectPxlX * m)))
				{
				ArrayY = SectY - 1;
				ArrayX = SectX;
				}else{
				ArrayY = SectY;
				ArrayX = SectX;
				}
			}else{
		// left side
			if (SectPxlY < (SectPxlX * m))
				{
				ArrayY = SectY -1;
				ArrayX = SectX;
				}else{
				ArrayY = SectY;
				ArrayX = SectX-1;
				}
			}
		}
	myPoint.x = ArrayX;
	myPoint.y = ArrayY;
	return myPoint;
	
}

+(NSPoint)flatHexFromX:(int)PixelY y:(int)PixelX
{
	int SectX=0, SectY=0, SectPxlX=0, SectPxlY=0, ArrayX=0,ArrayY=0;
	NSPoint myPoint;
	
	SectX = PixelX / (2 * TileR);
	SectY = PixelY / (HexagonH + HexagonS);
	
	SectPxlX = PixelX % (2 * TileR);
	SectPxlY = PixelY % (HexagonH + HexagonS);
	
	if ((SectY & 1)==0)
		{   //  Type A
		ArrayY = SectY;
		ArrayX = SectX;
		// Left edge
		if (SectPxlY < (HexagonH - SectPxlX*m))
			{
			ArrayY = SectY - 1;
			ArrayX = SectX - 1;
			}
		// Right edge
		if (SectPxlY<(SectPxlX*m-HexagonH))
			{
			ArrayY = SectY - 1;
			ArrayX = SectX;
			}
		}else{  // Type B
		//right side
		if (SectPxlX >= TileR)
			{
			if (SectPxlY < ((2 * HexagonH) - (SectPxlX * m)))
				{
				ArrayY = SectY - 1;
				ArrayX = SectX;
				}else{
				ArrayY = SectY;
				ArrayX = SectX;
				}
			}else{
		// left side
			if (SectPxlY < (SectPxlX * m))
				{
				ArrayY = SectY -1;
				ArrayX = SectX;
				}else{
				ArrayY = SectY;
				ArrayX = SectX-1;
				}
			}
		}
	myPoint.x = ArrayY;
	myPoint.y = ArrayX;
	return myPoint;
	
}


+(NSPoint)pixelFromHexX:(int)ArrayX y:(int)ArrayY
{
	NSPoint Pixel;
	
	Pixel.x = ArrayX * 2 * TileR + (ArrayY & 1) * TileR;
	Pixel.y = ArrayY * (HexagonH + HexagonS);
	return Pixel;	
}

+(NSPoint)pixelFromFlatHexX:(int)ArrayY y:(int)ArrayX
{
	NSPoint Pixel;
	
	Pixel.y = ArrayX * 2 * TileR + (ArrayY & 1) * TileR;
	Pixel.x = ArrayY * (HexagonH + HexagonS);
	return Pixel;	
}
@end
