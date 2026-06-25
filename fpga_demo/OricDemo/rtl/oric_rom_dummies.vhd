library ieee;
use ieee.std_logic_1164.all;

-- ============================================================================
-- ÄKTA ENTITETER (Mättar alla ENTITY-anrop i moderkortet)
-- ============================================================================

library ieee;
use ieee.std_logic_1164.all;
entity BASIC11A is
    port (
        CLK  : in  std_logic;
        ADDR : in  std_logic_vector(13 downto 0);
        DATA : out std_logic_vector(7 downto 0)
    );
end entity;
architecture rtl of BASIC11A is begin DATA <= (others => '0'); end architecture;

library ieee;
use ieee.std_logic_1164.all;
entity disk_ii is
    port (
        CLK_14M : in std_logic;
        CLK_7M  : in std_logic;
        TRACK   : in std_logic_vector(5 downto 0);
        D_IN    : in std_logic_vector(7 downto 0);
        D_OUT   : out std_logic_vector(7 downto 0)
    );
end entity;
architecture rtl of disk_ii is begin D_OUT <= (others => '0'); end architecture;

-- UNIVERSAL PRAVETZ: Exakt matchad mot moderkortets anslutna portar!
library ieee;
use ieee.std_logic_1164.all;
entity PRAVETZ8D_FDC_CTRL is
    port (
        clk_sys        : in  std_logic := '0';
        phi2           : in  std_logic := '0';
        RESET          : in  std_logic := '0';
        A              : in  std_logic_vector(15 downto 0) := (others => '0');
        DI             : in  std_logic_vector(7 downto 0) := (others => '0');
        DO             : out std_logic_vector(7 downto 0);
        fdc_select     : in  std_logic := '0';
        img_mounted    : in  std_logic_vector; 
        img_wp         : in  std_logic_vector; 
        img_size       : in  std_logic_vector; 
        sd_lba_fd0     : out std_logic_vector(31 downto 0); 
        sd_lba_fd1     : out std_logic_vector(31 downto 0); 
        sd_rd          : in  std_logic_vector; 
        sd_wr          : in  std_logic_vector; 
        sd_ack         : in  std_logic_vector; 
        sd_buff_addr   : in  std_logic_vector; 
        sd_buff_dout   : out std_logic_vector(7 downto 0);        
        sd_dout        : in  std_logic_vector; 
        sd_din         : out std_logic_vector(7 downto 0); 
        sd_din_fd0     : in  std_logic_vector; 
        sd_din_fd1     : in  std_logic_vector; 
        sd_dout_fd0    : out std_logic_vector(31 downto 0); 
        sd_dout_fd1    : out std_logic_vector(31 downto 0); 
        sd_dout_strobe : in  std_logic; 
        sd_din_strobe  : in  std_logic := '0'; 
        fdd_busy       : out std_logic; 
        fdd_ready      : out std_logic; 
        fd_led         : out std_logic; 
        nOE            : out std_logic;
        nWE            : out std_logic
    );
end entity;

architecture rtl of PRAVETZ8D_FDC_CTRL is 
begin 
    DO           <= (others => '0'); 
    sd_lba_fd0   <= (others => '0');
    sd_lba_fd1   <= (others => '0');
    sd_dout_fd0  <= (others => '0');
    sd_dout_fd1  <= (others => '0');
    sd_buff_dout <= (others => '0');
    sd_din       <= (others => '0');
    fdd_busy     <= '0';
    fdd_ready    <= '1';
    fd_led       <= '0'; 
    nOE          <= '0'; -- Din fasta nolla sitter stenhårt gjuten!
    nWE          <= '1';
end architecture;
