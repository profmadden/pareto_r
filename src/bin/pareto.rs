use pareto_r;
use argh::FromArgs;



#[derive(FromArgs)]
/// Pareto optimal set finding example.
struct ParArgs {
    #[argh(option, short = 'f')]
    /// filename
    filename: Option<String>,
}

pub fn main() {
    println!("Pareto optimization sample code");
    println!("This program generates a PostScript, which can be converted to PDF with GhostScript.");

    let arguments: ParArgs = argh::from_env();
    match arguments.filename {
        Some(filename) => {
            println!("Reading from {}", filename);
            let mut p = pareto_r::ParetoProblem::from_file(&filename);
            p.solve();
            let output = filename.clone() + ".ps";
            p.generate_ps(output, 10.0, 10.0);
        },
        _ => {println!("No input file specified!");}
    }
}