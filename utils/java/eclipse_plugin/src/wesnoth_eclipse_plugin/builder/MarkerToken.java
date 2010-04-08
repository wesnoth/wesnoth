package wesnoth_eclipse_plugin.builder;

import java.util.StringTokenizer;

import org.eclipse.core.resources.IMarker;

/**
 * @author Timotei Dolean
 */
public class MarkerToken{
	private MarkerTokenType type_ = MarkerTokenType.INFO;
	private String message_ = "";
	private int line_ ;
	private int columnStart_;
	private int columnEnd_;

	public MarkerToken(MarkerTokenType type,String message,int line, int columnStart,int columnEnd)
	{
		type_  = type;
		message_ = message;
		line_ = line;
		columnStart_ = columnStart;
		columnEnd_ = columnEnd;
	}

	/**
	 * Parses the current line and returns a marker token
	 * Current used format: Type#line:column#message
	 * @param line the line to parse
	 * @return
	 */
	public static MarkerToken parseToken(String line)
	{
		// severity#Line{:columnStart-columnEnd}#message
		StringTokenizer tokenizer = new StringTokenizer(line,"#");
		try{
			int lineIndex=1,columnIndexStart=0,columnIndexEnd=0; 
			MarkerTokenType type = MarkerTokenType.valueOf(tokenizer.nextToken().toUpperCase());

			if (tokenizer.countTokens() > 1) // we have the line+column indexes
			{
				String[] tmp = tokenizer.nextToken().split(":");

				if (tmp.length >0)
					lineIndex = Integer.parseInt(tmp[0]);
				if (tmp.length > 1)
				{
					// get columnstart-columnEnd
					String[] colTmp = tmp[1].split("-");
					columnIndexStart = Integer.parseInt(colTmp[0]);
					if(colTmp.length > 1)
						columnIndexEnd = Integer.parseInt(colTmp[1]);
				}
			}
			return new MarkerToken(type, tokenizer.nextToken(), lineIndex,columnIndexStart,columnIndexEnd);
		}
		catch (Exception e) {
			e.printStackTrace();
			return null;
		}
	}

	/** Returns the of the marker
	 * @return the line
	 */
	public int getLine() {
		return line_;
	}
	/** Returns the message of the marker
	 * @return the message of the marker
	 */
	public String getMessage() {
		return message_;
	}
	/** Returns the column start index of the marker
	 * @return the column_
	 */
	public int getColumnStart() {
		return columnStart_;
	}
	/** Returns the column end index of the marker
	 * @return the columnEnd_
	 */
	public int getColumnEnd() {
		return columnEnd_;
	}
	/**
	 * @return the type of the marker
	 */
	public MarkerTokenType getType() {
		return type_;
	}
}
enum MarkerTokenType
{
	ERROR, WARNING, INFO;
	public int toMarkerSeverity()
	{
		if (this == ERROR)
			return IMarker.SEVERITY_ERROR;
		else if (this == WARNING)
			return IMarker.SEVERITY_WARNING;
		
		return IMarker.SEVERITY_INFO;
	}
}