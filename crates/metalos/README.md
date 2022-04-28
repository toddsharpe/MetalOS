Generate bindings with Bindgen https://rust-lang.github.io/rust-bindgen/command-line-usage.html

```
bindgen metalos.h -o src/bindings.rs --use-core --ctypes-prefix=c_types
```

binding generation can be automated but that requires target to have clang (which we dont want).

But here's how

cargo.toml:
```
[build-dependencies]
bindgen = "*"
```

build.rs

```
    let bindings = bindgen::Builder::default()
        .header("metalos.h")
        .use_core()
        .ctypes_prefix("c_types")
        .generate()
        .expect("Unable to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
```