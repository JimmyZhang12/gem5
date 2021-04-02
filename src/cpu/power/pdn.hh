class pdn{
    public:
        pdn(double _L, double _C, double _R, double _VDC, double _CLK);

        double get_voltage(double current);
        double get_current(double power);
        void clk_throttle(double throttled_CLK);
        void print_params();

    private:
        double vout_2_cycle_ago;
        double vout_1_cycle_ago;
        double iout_1_cycle_ago;
        double L;
        double C;
        double R;
        double VDC;
        double CLK;
        double LmulC;
        double LdivR;
        double ts;

    
};