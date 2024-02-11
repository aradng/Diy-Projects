def cie1931(L):
    L = L * 103.0
    if L <= 8:
        return L / 903.3
    else:
        return ((L + 16.0) / 119.0) ** 3


def gen_cie(
    file,
    input_size=255,
    output_size=255,
    int_type="const uint32_t",
    table_name="cie",
):
    x = range(0, int(input_size + 1))
    y = [round(cie1931(float(L) / input_size) * output_size) for L in x]
    file.write("%s %s[%d] = {\n" % (int_type, table_name, input_size + 1))
    f.write("\t")
    for i, L in enumerate(y):
        f.write("%d, " % int(L))
        if i % 10 == 9:
            f.write("\n\t")
    f.write("\n};\n\n")


with open("esp_rgb/cie1931.h", "w") as f:
    f.write("// CIE1931 correction table\n")
    f.write("// Automatically generated\n\n")
    gen_cie(f, 255, 1023, table_name="cie")
    gen_cie(f, 99, 1023, table_name="cie_100")
