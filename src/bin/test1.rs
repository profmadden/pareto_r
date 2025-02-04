use pareto;

pub fn main() {
    println!("Pareto optimization sample code");
    println!("This program generates pareto.ps, which can be converted to PDF with GhostScript.");

    let mut p2 = pareto::ParetoProblem::new(2);
    p2.add_point(500, vec![50.0, 50.0]);
    p2.add_point(99,vec![30.0, 30.0]);
    p2.add_point(100, vec![10.0, 20.0]);
    p2.add_point(101, vec![20.0, 10.0]);
    p2.add_point(33, vec![15.0, 15.0]);

    p2.solve();
    p2.generate_ps("pareto.ps".to_string(), 20.0, 50.0);
    for p in &p2.points {
        println!("Point {} has rank {}", p.tag, p.rank);
    }
}