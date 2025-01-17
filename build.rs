fn main() {
    cc::Build::new()
        .file("src/pareto.c")
        .compile("pareto");
    println!("cargo:rerun-if-changed=src/pareto.c");
}
