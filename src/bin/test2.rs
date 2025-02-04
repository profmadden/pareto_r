use pareto;

pub fn main() {
    println!("Pareto optimization sample code");

    let mut problem = pareto::ParetoProblem::new(2);
    for x in 0..10 {
        for y in 0..10 {
            problem.add_point(x*10 + y, vec![x as f32 * 10.0, y as f32 * 10.0]);
        }
    }

    problem.solve();
    problem.generate_ps("grid.ps".to_string(), 20.0, 50.0);
}