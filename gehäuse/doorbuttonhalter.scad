$fn=200;

x = 70;
y = 25;
z = 50;


union(){
difference(){
	cube([x,y,z]);
	
	difference(){
		cube([y/2,y/2,z]);
		translate([12.5,12.5,0]) cylinder(h=z, r=y/2);
	}
	difference(){
		translate([x-y/2,0,0]) cube([y/2,y/2,z]);
		translate([x-y/2,y/2,0]) cylinder(h=z, r=y/2);
	}

	//Taster links
	translate([y/2,y/2,z-2]) cylinder(h=2, r=4.5);
	translate([12.5,12.5,40]) cylinder(h=8, r=11);
	translate([15,15,40]) cylinder(h=8, r=y/2);
	translate([12.5,12.5,2]) cylinder(h=36, r=11);
	translate([15,15,2]) cylinder(h=36, r=y/2);
	translate([y/2, y/2, 2]) difference(){
		cube([y/2,y/2,z-4]);
		cylinder(h=z-4, r=y/2);
	}
	translate([y/2,y/2,38]) intersection(){
		translate([-11/2,-12/2,0]) cube([11,12,2]);
		cylinder(h=2, r=6);
		
	}

	//Taster rechts
	translate([x-y/2,y/2,z-2]) cylinder(h=2, r=4.5);
	translate([x-12.5,12.5,40]) cylinder(h=8, r=11);
	translate([x-15,15,40]) cylinder(h=8, r=y/2);
	translate([x-12.5,12.5,2]) cylinder(h=36, r=11);
	translate([x-15,15,2]) cylinder(h=36, r=y/2);
	translate([x-y/2-0.01,y/2, 2]) rotate([0,0,90]) difference(){
		cube([y/2,y/2,z-4]);
		cylinder(h=z-4, r=y/2);
	}
	translate([x-y/2,y/2,38]) intersection(){
		translate([-11/2,-12/2,0]) cube([11,12,2]);
		cylinder(h=2, r=6);
		
	}

	//Frontfenster
	translate([x/2-(x-50)/2,2,2]) cube([x-50,y,z-4]);
	translate([x/2-(x-50)/2+2,0,4]) cube([x-50-4,2,z-8]);	
	echo("Outer: ", x-50-4, " x ", z-8);
	echo("Inner: ", x-50, " x ", z-4);

	//Kabelauslass
	translate([x/2-10, y-2.5, 0]) cube([20,2.5,2]);
}
translate([-10+0.1,y-2,0]) rotate([0,-90,-90]) difference(){
	cube([z,10,2]);
	difference(){
		translate([-2.5,-2.5,0]) cube([5,5,2]);
		translate([2.5,2.5,0]) cylinder(h=2.1,r=2.5);
	}
	translate([z,0,0]) rotate([0,0,90]) difference(){
		translate([-2.5,-2.5,0]) cube([5,5,2]);
		translate([2.5,2.5,0]) cylinder(h=2.1,r=2.5);
	}
	translate([5,5,0]) cylinder(h=2, d1=6, d2=3);
	translate([z-5,5,0]) cylinder(h=2, d1=6, d2=3);
}
translate([x-0.1,y-2,0]) rotate([0,-90,-90]) difference(){
	cube([z,10,2]);
	translate([0,10,0]) rotate([0,0,-90]) difference(){
		translate([-2.5,-2.5,0]) cube([5,5,2]);
		translate([2.5,2.5,0]) cylinder(h=2.1,r=2.5);
	}
	translate([z,10,0]) rotate([0,0,180]) difference(){
		translate([-2.5,-2.5,0]) cube([5,5,2]);
		translate([2.5,2.5,0]) cylinder(h=2.1,r=2.5);
	}
	translate([5,5,0]) cylinder(h=2, d1=6, d2=3);
	translate([z-5,5,0]) cylinder(h=2, d1=6, d2=3);
}
}