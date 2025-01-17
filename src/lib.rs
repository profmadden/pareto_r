use cty;
use pstools_r;

extern "C" {
    pub fn pareto_hello();
    // pub unsafe fn pareto_set_size();
    // pub unsafe fn pareto_set_nv();
}
extern "C" {
    pub fn pareto_set_size(num_points: cty::c_int, num_dimensions: cty::c_int);
}
extern "C" {
    pub fn pareto_set_nv(pn: cty::c_int, dim: cty::c_int, v: cty::c_float);
    pub fn pareto_free();
    pub fn pareto_solve();
    pub fn pareto_get_rank(pn: cty::c_int) -> cty::c_int;
    pub fn pareto_get_id(pn: cty::c_int) -> cty::c_int;
    pub fn pareto_set_id(pn: cty::c_int, tag: cty::c_int);
}

pub struct ParetoPoint {
    pub v: Vec<cty::c_float>,
    pub tag: cty::c_int,
    pub rank: cty::c_int,
}

pub struct ParetoProblem {
    pub points: Vec<ParetoPoint>,
    pub dimensions: usize,
}

impl ParetoProblem {
    pub fn new(dimensions: usize) -> ParetoProblem {
	    ParetoProblem {
    	    points: Vec::new(),
            dimensions,
    	}
    }
    pub fn add_point(&mut self, tag: cty::c_int, values: Vec<cty::c_float>) {
        self.points.push(ParetoPoint{
            v: values.clone(),
            tag,
            rank: 0,
        });
    }
    pub fn solve(&mut self) {
        println!("Solving the pareto problem!");
        unsafe {
            pareto_set_size(self.points.len() as cty::c_int, self.dimensions as cty::c_int);
            for i in 0..self.points.len() {
                for d in 0..self.dimensions {
                    pareto_set_nv(i as cty::c_int, d as cty::c_int, self.points[i].v[d] as cty::c_float);
                    // pareto_set_id(i as cty::c_int, self.points[i].tag);
                }
            }
            println!("About to call the solver!");
            pareto_solve();
            println!("Solution complete");
            for i in 0..self.points.len() {
                let r = pareto_get_rank(i as cty::c_int);
                let id = pareto_get_id(i as cty::c_int);

                println!("Item {} id {} -> rank {}", i, id, r);
                self.points[id as usize].rank = r;
            }

            // pareto_free();
        }
    }

    pub fn generate_ps(&self, filename: String, scale: f32, border: f32) {
        let mut pst = pstools_r::PSTool::new();
        let mut bounds = pstools_r::bbox::BBox::new();


        pst.set_color(1.0, 0.0, 0.0, 1.0);
        pst.set_scale(scale);
        pst.set_border(border);

        for p in &self.points {
            bounds.addpoint(p.v[0], p.v[1]);
            pst.add_box(p.v[0], p.v[1], p.v[0] + 1.0, p.v[1] + 1.0);
        }
        pst.set_color(0.0, 0.0, 0.0, 1.0);
        for p in &self.points{
            pst.add_text(p.v[0], p.v[1], format!("{}:{}", p.tag, p.rank));
        }
        pst.generate(filename);
    }

    pub fn hello(&self) {
    	println!("Hello from a RUST interface Pareto problem.");
    }
}
