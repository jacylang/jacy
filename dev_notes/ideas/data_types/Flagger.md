# Flagger data type

> Note: Macro syntax is not fully described

```cpp
#declareFlagger($name: ident, $type: type, $($flag: ident), *) {
    struct $name<$type> {
        $type bits
        enum Flags {
            $(
                $flag,
            )*
        }
    }
}
```