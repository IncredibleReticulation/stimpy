

class Status
{
	public:
		static const int SMTP_SRV_RDY         = 220;
		static const int SMTP_SRV_CLOSE       = 221;
		static const int SMTP_ACTION_COMPLETE = 250;
		static const int SMTP_BEGIN_MSG       = 354;
		static const int SMTP_SRV_NOT_AVAIL   = 421;
		static const int SMTP_OUT_TIMEOUT     = 447;
		static const int SMTP_CMD_SNTX_ERR    = 500;		
};
