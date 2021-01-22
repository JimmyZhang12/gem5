
L = 0
C = 0
R = 0
VDC = 0
CLK = 0
vout_2_cycle_ago = 0
vout_1_cycle_ago = 0
iout_1_cycle_ago = 0 

def init_PDN(l, c, r, vdc, clk):
    global L 
    global C
    global R
    global VDC 
    global CLK 
    global vout_2_cycle_ago 
    global vout_1_cycle_ago 
    global iout_1_cycle_ago 
    L = l
    C = c
    R = r
    VDC = vdc
    CLK = clk
    vout_2_cycle_ago = vdc
    vout_1_cycle_ago = vdc
    iout_1_cycle_ago = 0

def test():
    global L
    global C
    global R
    global VDC
    print("******************pdn object call test!")
    print("***LCR = ", L," ",C," ",R, " ", VDC)

def get_volt(current):
    global L 
    global C
    global R
    global VDC 
    global CLK 
    global vout_2_cycle_ago 
    global vout_1_cycle_ago 
    global iout_1_cycle_ago 
    ts = 1/CLK
    LmulC = L*C
    LdivR = L/R
    vout = VDC*ts**2/(LmulC) \
        + vout_1_cycle_ago*(2 - ts/(LdivR)) \
        + vout_2_cycle_ago*(ts/(LdivR) \
        - 1 - ts**2/(LmulC)) \
        - current*R*ts**2/(LmulC) \
        - (1/C)*ts*(current - iout_1_cycle_ago)
        
    vout_2_cycle_ago = vout_1_cycle_ago
    vout_1_cycle_ago = vout
    iout_1_cycle_ago = current
    return vout
    